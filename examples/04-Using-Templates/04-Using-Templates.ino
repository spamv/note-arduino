//
// Copyright 2019 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
//
// This example does the same function as the basic "Using Library" example, but demonstrates
// how best to use the Notecard API when your app uses either a) higher-frequency sampling, or
// b) binary samples.
//
// When using the standard Notecard "note.add" request for sending data to the service through
// outbound queues (notefiles ending in .qo), the Notecard limits the number of "pending notes"
// (the number of notes waiting to be sent to the service) to 100, because under these common
// circumstances the pending notes are all buffered in memory.
//
// When a customer application generates data at a higher frequency, or a longer sync period,
// such that more than 100 notes will be "waiting" to be uploaded, the developer should use
// a feature known as the "Note Template".	An application using a Note Template declares
// once, up-front, what the JSON object will 'look like' in terms of JSON fields, data types,
// and data length.	 By doing this, the Notecard can store the data for each "note.add"
// directly into a region of Flash storage permitting more than 100 notes to be pending
// for subsequent upload.
//
// This example shows how a developer would declare and set a template, both for their own
// JSON object as well as for their own binary payload.	 (If a binary payload is not needed,
// it can simply be eliminated from the example.)
//

#include <Notecard.h>
#include <stdlib.h>

// Define your Notecard serial port, else comment this out for I2C

//#define notecard Serial2

// This is the unique Product Identifier for your device.  This Product ID tells the Notecard what
// type of device has embedded the Notecard, and by extension which vendor or customer is in charge
// of "managing" it.  In order to set this value, you must first register with notehub.io and
// "claim" a unique product ID for your device.	 It could be something as simple as as your email
// address in reverse, such as "com.gmail.smith.lisa.test-device" or "com.outlook.gates.bill.demo"

#define myProductID "org.coca-cola.soda.vending-machine.v2"

// A sample binary object, just for binary payload simulation
struct myBinaryPayload {
	double temp;
	double voltage;
};

// One-time Arduino initialization
void setup() {

	// Set up for debug output.	 If you open Arduino's serial terminal window, you'll be able to
	// watch JSON objects being transferred to and from the Notecard for each request.	On most
	// Arduino devices, Arduino's serial debug output is on the "Serial" device at 115200.
	// If you don't wish to see the Notecard's debug output, or if your device doesn't have
	// any debug output port, just comment out these lines by preceding them with //
	Serial.begin(115200);
	NoteSetDebugOutputStream(Serial);

	// Initialize the physical I/O channel to the Notecard
#ifdef notecard
	NoteInitSerial(notecard, 9600);
#else
	NoteInitI2C();
#endif

	// "NoteNewRequest()" uses the bundled "J" json package to allocate a "req", which is a JSON object
	// for the request to which we will then add Request arguments.	 The function allocates a "req"
	// request structure using malloc() and initializes its "req" field with the type of request.
	J *req = NoteNewRequest("service.set");

	// This command (required) causes the data to be delivered to the Project on notehub.io that has claimed
	// this Product ID.	 (see above)
	JAddStringToObject(req, "product", myProductID);

	// This command determines how often the Notecard connects to the service.
	JAddStringToObject(req, "mode", "periodic");
	JAddNumberToObject(req, "minutes", 5);

	// Issue the request, telling the Notecard how and how often to access the service.
	// This results in a JSON message to Notecard formatted like:
	//	   { "req"	   : "service.set",
	//		 "product" : myProductID,
	//		 "mode"	   : "continuous"
	//	   }
	// Note that NoteRequest() always uses free() to release the request data structure, and it
	// returns "true" if success and "false" if there is any failure.
	NoteRequest(req);

	// Create a template note that we will register.  This template note will look "similar" to
	// the notes that will later be added with note.add, in that the data types are used to
	// intuit what the ultimate field data types will be, and their maximum length.
	req = NoteNewRequest("note.template");
	if (req != NULL) {

		// Create the body for a template that will be used to send notes below
		J *body = JCreateObject();
		if (body != NULL) {

			// Define the JSON template
			JAddStringToObject(body, "status", "AAAAAAAAAAAA");		// maximum string length
			JAddNumberToObject(body, "temp", 1.1);					// floating point (double)
			JAddNumberToObject(body, "voltage", 1.1);				// floating point (double)
			JAddNumberToObject(body, "count", 1);					// integer

			// Add it to the template note
			JAddItemToObject(req, "body", body);
		}

		// Create a template of the payload that will be used to send notes below
		myBinaryPayload binaryData = {0};
		JAddBinaryToObject(req, "payload", &binaryData, sizeof(binaryData));

		// Register the template in the output queue notefile
		JAddStringToObject(req, "file", "sensors.qo");
		NoteRequest(req);
	}


}

// In the Arduino main loop which is called repeatedly, add outbound data every 15 seconds
void loop() {

	// Simulate an event counter of some kind
	static unsigned eventCounter = 0;
	eventCounter = eventCounter + 1;

	// Rather than simulating a temperature reading, use a Notecard request to read the temp
	// from the Notecard's built-in temperature sensor.	 We use NoteRequestResponse() to indicate
	// that we would like to examine the response of the transaction.  This method takes a "request" JSON
	// data structure as input, then processes it and returns a "response" JSON data structure with
	// the response.  Note that because the Notecard library uses malloc(), developers must always
	// check for NULL to ensure that there was enough memory available on the microcontroller to
	// satisfy the allocation request.
	double temperature = 0;
	J *rsp = NoteRequestResponse(NoteNewRequest("card.temp"));
	if (rsp != NULL) {
		temperature = JGetNumber(rsp, "value");
		NoteDeleteResponse(rsp);
	}

	// Add an indication of temperature status
	if (temperature > 26.67)	// 80F
		JAddStringToObject(rsp, "status", "hot");
	if (temperature < -6.67)	// 20F
		JAddStringToObject(rsp, "status", "cold");

	// Do the same to retrieve the voltage that is detected by the Notecard on its V+ pin.
	double voltage = 0;
	rsp = NoteRequestResponse(NoteNewRequest("card.voltage"));
	if (rsp != NULL) {
		voltage = JGetNumber(rsp, "value");
		NoteDeleteResponse(rsp);
	}

	// Add a binary data structure to the simulation
	myBinaryPayload binaryData;
	binaryData.temp = temperature;
	binaryData.voltage = voltage;

	// Enqueue the measurement to the Notecard for transmission to the Notehub, adding the "start"
	// flag for demonstration purposes to upload the data instantaneously, so that if you are looking
	// at this on notehub.io you will see the data appearing 'live'.)
	J *req = NoteNewRequest("note.add");
	if (req != NULL) {
		JAddStringToObject(req, "file", "sensors.qo");
		J *body = JCreateObject();
		if (body != NULL) {
			JAddNumberToObject(body, "temp", temperature);
			JAddNumberToObject(body, "voltage", voltage);
			JAddNumberToObject(body, "count", eventCounter);
			JAddBinaryToObject(req, "payload", &binaryData, sizeof(binaryData));
			JAddItemToObject(req, "body", body);
		}
		NoteRequest(req);
	}

	// Delay between measurements
	delay(5*1000);		// 5 seconds

}
