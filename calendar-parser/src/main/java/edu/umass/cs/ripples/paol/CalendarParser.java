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
	  
		if(args.length != 1) {
			println("Exactly one calendar (no spaces) should be specified");
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
		if((calKey = sumryToIdMap.get(args[0])) == null) {
			println("Calendar '" + args[0] + "' was not found. Here are the available calendars:");
			for(String key : sumryToIdMap.keySet())
				println(key);
			System.exit(1);
		}
		
		semester = args[0];
		println("Calendar '" + semester + "' was found. Writing cron lines\n");
		Calendar calendar = client.calendars().get(calKey).execute();
		// create file and BufferedWriter
		File cronLines = new File(CRON_TEMP);
		if(cronLines.exists())
			cronLines.delete();
		cronLines.createNewFile();
		BufferedWriter writer = new BufferedWriter(new FileWriter(cronLines));
        writeEvents(calendar, writer);
		writer.close();
		System.exit(0);

      } catch (IOException e) {
        System.err.println(e.getMessage());
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

  private static void writeEvents(Calendar calendar, BufferedWriter writer) throws IOException {
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
		println(cronLine(e, semester));
		System.out.println();
		writeLineToFile(writer, cronLine(e, semester));
	}
	if(events.size() == 0)
		println("Warning: No events found within the scan period. cron_temp.txt should be empty");
	else
		println("Finished writing lines to file");
  }
  
  private static String cronLine(Event e, String sem) {
	if(e.getStart().getDateTime() == null) {
		System.err.println("[CalendarParser] Tried to parse all-day event");
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
	return "#" + min + " " + hr + " " + dayOfMon + " " + (mon+1) + " * " + (year+1900) + " " + cScriptLoc + " " + sem + " " + course + " " + dur;
  }
  
  private static void writeLineToFile(BufferedWriter writer, String line) throws IOException {
	writer.write(line);
	writer.newLine();
  }
  
  private static void println(String toPrint) {
	System.out.println("[CalendarParser] " + toPrint);
  }

}