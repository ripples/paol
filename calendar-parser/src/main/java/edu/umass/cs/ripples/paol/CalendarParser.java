/*
 * PAOL Calendar Parser
 *
 * This program reads from a user's calendar and creates cron jobs for capturing
 * lectures. The code is adapted from Google's calendar command line example, which
 * can be found here:
 * https://code.google.com/p/google-api-java-client/source/browse/calendar-cmdline-sample/src/main/java/com/google/api/services/samples/calendar/cmdline/CalendarSample.java?repo=samples
 *
 * Refer to the link below for licensing information:
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Ryan Szeto
 */

package edu.umass.cs.ripples.paol;

import com.google.api.client.auth.oauth2.Credential;
import com.google.api.client.extensions.java6.auth.oauth2.AuthorizationCodeInstalledApp;
import com.google.api.client.extensions.java6.auth.oauth2.FileCredentialStore;
import com.google.api.client.extensions.jetty.auth.oauth2.LocalServerReceiver;
import com.google.api.client.googleapis.auth.oauth2.GoogleAuthorizationCodeFlow;
import com.google.api.client.googleapis.auth.oauth2.GoogleClientSecrets;
import com.google.api.client.googleapis.batch.BatchRequest;
import com.google.api.client.googleapis.batch.json.JsonBatchCallback;
import com.google.api.client.googleapis.javanet.GoogleNetHttpTransport;
import com.google.api.client.googleapis.json.GoogleJsonError;
import com.google.api.client.http.HttpHeaders;
import com.google.api.client.http.HttpTransport;
import com.google.api.client.json.JsonFactory;
import com.google.api.client.json.jackson2.JacksonFactory;
import com.google.api.client.util.DateTime;
import com.google.api.client.util.Lists;
import com.google.api.services.calendar.CalendarScopes;
import com.google.api.services.calendar.model.Calendar;
import com.google.api.services.calendar.model.CalendarList;
import com.google.api.services.calendar.model.Event;
import com.google.api.services.calendar.model.EventDateTime;
import com.google.api.services.calendar.model.Events;
import com.google.api.services.calendar.model.CalendarListEntry;

import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collections;
import java.util.Date;
import java.util.TimeZone;
import java.util.List;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Map;
import java.util.HashMap;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.StringTokenizer;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.FileNotFoundException;

/**
 * @author Yaniv Inbar
 * @author Ryan Szeto
 */
public class CalendarParser {

  /**
   * Be sure to specify the name of your application. If the application name is {@code null} or
   * blank, the application will log a warning. Suggested format is "MyCompany-ProductName/1.0".
   */
  private static final String APPLICATION_NAME = "PAOL Calendar Parser";
  
  /** Global instance of the HTTP transport. */
  private static HttpTransport HTTP_TRANSPORT;

  /** Global instance of the JSON factory. */
  private static final JsonFactory JSON_FACTORY = new JacksonFactory();

  private static com.google.api.services.calendar.Calendar client;
  
  private static final String CAP_SCRIPT = "/home/paol/paol-code/scripts/capture/fullCapture.sh";
  private static final String CRON_TEMP = "/home/paol/paol-code/cron_temp.txt";
  private static final String SEM_DATES = "/home/paol/paol-code/semesterDates.txt";
  
  private static String semester = "default";
  
  // How many seconds before and after lecture to start and end capture
  private static final long bufferStart = 30;
  private static final long bufferEnd = 120;
  
  // How many days to look ahead
  private static final int scanPeriod = 3;

  /** Authorizes the installed application to access user's protected data. */
  private static Credential authorize() throws Exception {
    // load client secrets
    GoogleClientSecrets clientSecrets = GoogleClientSecrets.load(JSON_FACTORY,
        new InputStreamReader(CalendarParser.class.getResourceAsStream("/client_secrets.json")));
    if (clientSecrets.getDetails().getClientId().startsWith("Enter")
        || clientSecrets.getDetails().getClientSecret().startsWith("Enter ")) {
      println(
          "Enter Client ID and Secret from https://code.google.com/apis/console/?api=calendar "
          + "into calendar-cmdline-sample/src/main/resources/client_secrets.json");
      System.exit(1);
    }
    // set up file credential store; info is stored in ~/.credentials/calendar.json
    FileCredentialStore credentialStore = new FileCredentialStore(
        new File(System.getProperty("user.home"), ".credentials/calendar.json"), JSON_FACTORY);
    // set up authorization code flow
    GoogleAuthorizationCodeFlow flow = new GoogleAuthorizationCodeFlow.Builder(
        HTTP_TRANSPORT, JSON_FACTORY, clientSecrets,
        Collections.singleton(CalendarScopes.CALENDAR)).setCredentialStore(credentialStore).build();
    // authorize
    return new AuthorizationCodeInstalledApp(flow, new LocalServerReceiver()).authorize("user");
  }

  public static void main(String[] args) {
    try {
      try {
	  
		if(args.length != 2) {
			errPrintln("Usage: <program to run> <name of calendar>");
			System.exit(1);
		}

        // initialize the transport
        HTTP_TRANSPORT = GoogleNetHttpTransport.newTrustedTransport();

        // authorization
        Credential credential = authorize();

        // set up global Calendar instance
        client = new com.google.api.services.calendar.Calendar.Builder(
            HTTP_TRANSPORT, JSON_FACTORY, credential).setApplicationName(
            APPLICATION_NAME).build();

		// get calendars
		Map<String, String> sumryToIdMap = initSumryToIdMap();
		String calKey = null;
		String captureProgram = args[0];
		String calendarName = args[1];
		if((calKey = sumryToIdMap.get(calendarName)) == null) {
			println("Calendar '" + calendarName + "' was not found. Here are the available calendars:");
			for(String key : sumryToIdMap.keySet())
				println(key);
			System.exit(1);
		}

		println("Calendar '" + calendarName + "' was found. Writing cron lines");
		println("Setting semester\n");
		if(!setSemester(SEM_DATES))
			errPrintln("Failed to set semester");
		Calendar calendar = client.calendars().get(calKey).execute();
		// create file and BufferedWriter
		File cronLines = new File(CRON_TEMP);
		if(cronLines.exists())
			cronLines.delete();
		cronLines.createNewFile();
		BufferedWriter writer = new BufferedWriter(new FileWriter(cronLines));
        writeEvents(captureProgram, calendar, writer);
		writer.close();
		System.exit(0);

      } catch (IOException e) {
        errPrintln(e.getMessage());
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
    System.exit(1);
  }
  
  public static Map<String, String> initSumryToIdMap() throws IOException {
	HashMap<String, String> ret = new HashMap<String, String>();
	CalendarList feed = client.calendarList().list().execute();
	List<CalendarListEntry> entries = feed.getItems();
	for(CalendarListEntry e : entries)
		ret.put(e.getSummary(), e.getId());
	return ret;
  }

  private static void writeEvents(String captureProgram, Calendar calendar, BufferedWriter writer) throws IOException {
	Date start = new Date();
	Date end = new Date(start.getTime() + scanPeriod*24*3600*1000);
	DateTime startDT = new DateTime(start);
	DateTime endDT = new DateTime(end);
	Events feed = client.events().list(calendar.getId()).set("singleEvents", true)
			.set("orderBy", "startTime")
			.set("timeMin", startDT).set("timeMax", endDT).execute();
	List<Event> events = feed.getItems();
	for(Event e : events) {
		if(e.getStart().getDateTime() == null)
			continue;
		long eStartLong = e.getStart().getDateTime().getValue();
		long eEndLong = e.getEnd().getDateTime().getValue();
		long duration = (eEndLong - eStartLong)/1000;
		Date eventStart = new Date(eStartLong);
		println(e.getSummary());
		println("Start: " + eventStart.toString());
		println("Duration (s): " + duration);
		println("Writing cron job:");
		println(cronLine(captureProgram, e, semester));
		System.out.println();
		String crline = cronLine(captureProgram, e, semester);
		if(crline != null)
			writeLineToFile(writer, crline);
	}
	if(events.size() == 0)
		errPrintln("Warning: No events found within the scan period. cron_temp.txt should be empty");
	else
		println("Finished writing lines to file");
  }
  
  private static String cronLine(String captureProgram, Event e, String sem) {
	if(e.getStart().getDateTime() == null) {
		errPrintln("Tried to parse all-day event");
		return null;
	}
	long eStartLong = e.getStart().getDateTime().getValue();
	long eEndLong = e.getEnd().getDateTime().getValue();
	long duration = (eEndLong - eStartLong)/1000;
	Date sDate = new Date(eStartLong - bufferStart*1000);
	String courseTitle = e.getSummary();
	// Replace all spaces with underscores
	courseTitle = courseTitle.replace(" ", "_");
	// Find course list number, which will be the text before the colon in the course title
	// If no colon is found, use the whole course title as the "list number"
	int courseListNumEnd = courseTitle.length();
	int colonIdx = courseTitle.indexOf(":");
	if(colonIdx > 0) {
		// Course list number found
		courseListNumEnd = colonIdx;
	}
	String course = courseTitle.substring(0, courseListNumEnd);
	return cronLine(captureProgram, sDate.getMinutes(), sDate.getHours(), sDate.getDate(), sDate.getMonth(), sDate.getYear(), sem, course, duration+bufferStart+bufferEnd);
  }
  
  // m h  dom mon dow year   command
  private static String cronLine(String captureProgram, int min, int hr, int dayOfMon, int mon, int year, String sem, String course, long dur) {
	//return min + " " + hr + " " + dayOfMon + " " + (mon+1) + " * " + (year+1900) + " " + CAP_SCRIPT + " " + sem + " " + course + " " + dur;
	return min + " " + hr + " " + dayOfMon + " " + (mon+1) + " * " + captureProgram + " " + sem + " " + course + " " + dur;
  }
  
  private static void writeLineToFile(BufferedWriter writer, String line) throws IOException {
	writer.write(line);
	writer.newLine();
  }
  
  private static void println(String toPrint) {
	System.out.println("[CalendarParser] " + toPrint);
  }
  
  private static void errPrintln(String toPrint) {
	System.err.println("[CalendarParser] " + toPrint);
  }
  
  private static boolean setSemester(String datesLoc) throws IOException {
	try {
		File datesFile = new File(datesLoc);
		BufferedReader reader = new BufferedReader(new FileReader(datesFile));
		String line;
		while((line = reader.readLine()) != null) {
			StringTokenizer tokenizer = new StringTokenizer(line, " :");
			if(tokenizer.countTokens() != 3) {
				errPrintln("Malformated semester file");
				return false;
			}
			String sem = tokenizer.nextToken();
			String start = tokenizer.nextToken();
			String end = tokenizer.nextToken();
			if(todayIsInSemester(start, end)) {
				semester = sem;
				return true;
			}
		}
		errPrintln("Today is not within a semester");
		return false;
	} catch (FileNotFoundException e) {
		errPrintln(e.toString());
		return false;
	}
  }
  
  private static boolean todayIsInSemester(String startDay, String endDay) {
	try {
		Date today = new Date();
		String startTime = startDay + " 12:00 AM";
		String endTime = endDay + " 11:59 PM";
		SimpleDateFormat df = new SimpleDateFormat();
		Date s = df.parse(startTime);
		Date e = df.parse(endTime);
		return today.compareTo(s) >= 0 && today.compareTo(e) <= 0;
	} catch (ParseException e) {
		errPrintln(e.toString());
		System.exit(1);
	}
	return false;
  }

}
