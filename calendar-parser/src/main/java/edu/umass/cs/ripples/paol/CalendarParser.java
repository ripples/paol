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
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;

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

  static final java.util.List<Calendar> addedCalendarsUsingBatch = Lists.newArrayList();
  
  private static final java.util.Map<String, String> sumryToIdMap = new java.util.HashMap<String, String>();
  
  private static final String cScriptLoc = "/home/paol/paol-code/scripts/capture/fullCapture.sh";
  
  private static String semester = "Default";
  
  private static final String CRON_TEMP = "/home/paol/paol-code/cron_temp.txt";
  
  // How many seconds before lecture to start capture
  private static final long buffer = 300;
  
  // How many days to look ahead
  private static final int scanPeriod = 3;

  /** Authorizes the installed application to access user's protected data. */
  private static Credential authorize() throws Exception {
    // load client secrets
    GoogleClientSecrets clientSecrets = GoogleClientSecrets.load(JSON_FACTORY,
        new InputStreamReader(CalendarParser.class.getResourceAsStream("/client_secrets.json")));
    if (clientSecrets.getDetails().getClientId().startsWith("Enter")
        || clientSecrets.getDetails().getClientSecret().startsWith("Enter ")) {
      System.out.println(
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

        // initialize the transport
        HTTP_TRANSPORT = GoogleNetHttpTransport.newTrustedTransport();

        // authorization
        Credential credential = authorize();

        // set up global Calendar instance
        client = new com.google.api.services.calendar.Calendar.Builder(
            HTTP_TRANSPORT, JSON_FACTORY, credential).setApplicationName(
            APPLICATION_NAME).build();

        // run commands
        showCalendars();
        
		System.out.print("Enter the title of the calendar to read: ");
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		String summary = in.readLine();
		while(sumryToIdMap.get(summary) == null) {
			if(summary.equals("")) {
				System.out.println("Exiting");
				System.exit(0);
			}
			else {
				System.out.print("Could not find calendar. Enter another title: ");
				summary = in.readLine();
			}
		}
		semester = summary;
		Calendar calendar = client.calendars().get(sumryToIdMap.get(summary)).execute();
		// create file and BufferedWriter
		File cronLines = new File(CRON_TEMP);
		if(cronLines.exists())
			cronLines.delete();
		cronLines.createNewFile();
		BufferedWriter writer = new BufferedWriter(new FileWriter(cronLines));
        writeEvents(calendar, writer);
		writer.close();

      } catch (IOException e) {
        System.err.println(e.getMessage());
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
    System.exit(1);
  }

  private static void showCalendars() throws IOException {
    CalendarList feed = client.calendarList().list().execute();
	List<CalendarListEntry> entries = feed.getItems();
	System.out.println("Available calendars:");
	for(CalendarListEntry e : entries) {
		sumryToIdMap.put(e.getSummary(), e.getId());
		System.out.println(e.getSummary() + " - " + e.getId());
	}
  }

  private static void writeEvents(Calendar calendar, BufferedWriter writer) throws IOException {
    System.out.println("Below are the events from the calendar:\n");
	Date start = new Date();
	Date end = new Date(start.getTime() + scanPeriod*24*3600*1000);
	DateTime startDT = new DateTime(start);
	DateTime endDT = new DateTime(end);
    Events feed = client.events().list(calendar.getId()).set("singleEvents", true)
			.set("orderBy", "startTime")
			.set("timeMin", startDT).set("timeMax", endDT).execute();
	java.util.List<Event> events = feed.getItems();
	for(Event e : events) {
		if(e.getStart().getDateTime() == null)
			continue;
		long eStartLong = e.getStart().getDateTime().getValue();
		long eEndLong = e.getEnd().getDateTime().getValue();
		long duration = (eEndLong - eStartLong)/1000;
		Date eventStart = new Date(eStartLong);
		System.out.println(e.getSummary());
		System.out.println("Start: " + eventStart.toString());
		System.out.println("Duration (s): " + duration);
		System.out.println("Writing cron job:");
		System.out.println(cronLine(e, semester));
		System.out.println();
		writeLineToFile(writer, cronLine(e, semester));
	}
  }
  
  private static String cronLine(Event e, String sem) {
	if(e.getStart().getDateTime() == null) {
		System.err.println("Tried to parse all-day event");
		return null;
	}
	long eStartLong = e.getStart().getDateTime().getValue();
	long eEndLong = e.getEnd().getDateTime().getValue();
	long duration = (eEndLong - eStartLong)/1000;
	Date sDate = new Date(eStartLong - buffer*1000);
	return cronLine(sDate.getMinutes(), sDate.getHours(), sDate.getDate(), sDate.getMonth(), sDate.getYear(), sem, e.getSummary(), duration+2*buffer);
  }
  
  // m h  dom mon dow year   command
  private static String cronLine(int min, int hr, int dayOfMon, int mon, int year, String sem, String course, long dur) {
	return min + " " + hr + " " + dayOfMon + " " + (mon+1) + " * " + (year+1900) + " " + cScriptLoc + " " + sem + " " + course + " " + dur;
  }
  
  private static void writeLineToFile(BufferedWriter writer, String line) throws IOException {
	writer.write(line);
	writer.newLine();
  }

}