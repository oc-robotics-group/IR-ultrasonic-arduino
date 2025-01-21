import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;
 
Serial myPort; // Serial port object for communication with external hardware
//String manualPort = null; // Optional: Manually set a specific port (e.g., "COM10" or "/dev/ttyUSB0")
String manualPort = "/dev/ttyUSB0";
String angleData = ""; // Stores angle data received from serial input
String distanceData = ""; // Stores distance data received from serial input
String rawData = ""; // Raw data string from serial port
String statusMessage; // Displays if the object is in or out of range
float distancePixels; // Distance in pixels, based on the real-world distance
int parsedAngle, parsedDistance; // Parsed angle and distance values as integers
int commaIndex1 = 0; // Index for separating angle and distance in raw data
int commaIndex2 = 0;
PFont fontType;
 
void setup() {
  size(1200, 700); // Adjust to screen size
  smooth();
  
  // List all available serial ports
  String[] ports = Serial.list();
  println("Available ports:");
  for (int i = 0; i < ports.length; i++) {
    println(i + ": " + ports[i]);
  }
  
  if (manualPort != null) {
    println("Attempting to use manually set port: " + manualPort);
    try {
      myPort = new Serial(this, manualPort, 115200); // Open manually set port
      myPort.bufferUntil('.'); // Read data until a '.' character
      println("Successfully connected to manual port: " + manualPort);
      return; // Exit setup after successful connection
    } catch (Exception e) {
      println("Failed to open manual port: " + manualPort + ". Falling back to auto-detection.");
    }
  }
  
  // Auto-detection of available ports
  if (Serial.list().length > 0) {
    for (String port : ports) {
      try {
        myPort = new Serial(this, port, 115200); // Open the port
        println("Checking for Arduino on port: " + port);
        delay(1700); // Wait before attempting to read data
        if (myPort != null && myPort.available() > 0) {
          println("Found Arduino port: " + port);
          myPort.bufferUntil('.'); // Read data until a '.' character
          return;
        }
      } catch (Exception e) {
        println("Could not open port: " + port);
      }
    }
  }
  
  println("Arduino not found. Please check the connection.");
}
 
void draw() {
  fill(98, 245, 31);
  noStroke();
  fill(0, 4);
  rect(0, 0, width, height - height * 0.065);
  fill(98, 245, 31);
 
  drawRadarDisplay();
  drawDirectionLine();
  drawDetectedObject();
  displayText();
}
 
void serialEvent(Serial port) {
  try {
    rawData = port.readStringUntil('.');
    rawData = rawData.substring(0, rawData.length() - 1); // Remove trailing '.'
    commaIndex1 = rawData.indexOf(",");
    angleData = rawData.substring(0, commaIndex1);
    distanceData = rawData.substring(commaIndex1 + 1, rawData.length());
    parsedAngle = int(angleData);
    parsedDistance = int(distanceData);
  } catch (RuntimeException e) {
    e.printStackTrace();
  }
}
 
void drawRadarDisplay() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);
  
  arc(0, 0, width - width * 0.0625, width - width * 0.0625, PI, TWO_PI);
  arc(0, 0, width - width * 0.27, width - width * 0.27, PI, TWO_PI);
  arc(0, 0, width - width * 0.479, width - width * 0.479, PI, TWO_PI);
  arc(0, 0, width - width * 0.687, width - width * 0.687, PI, TWO_PI);
 
  line(-width / 2, 0, width / 2, 0);
  for (int angle = 30; angle <= 150; angle += 30) {
    line(0, 0, (-width / 2) * cos(radians(angle)), (-width / 2) * sin(radians(angle)));
  }
  popMatrix();
}
 
void drawDetectedObject() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  strokeWeight(9);
  stroke(255, 10, 10);
  distancePixels = parsedDistance * ((height - height * 0.1666) * 0.025);
  
  if (parsedDistance < 40) {
    line(
      distancePixels * cos(radians(parsedAngle)), 
      -distancePixels * sin(radians(parsedAngle)),
      (width - width * 0.505) * cos(radians(parsedAngle)), 
      -(width - width * 0.505) * sin(radians(parsedAngle))
    );
  }
  popMatrix();
}
 
void drawDirectionLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60);
  translate(width / 2, height - height * 0.074);
  line(0, 0, (height - height * 0.12) * cos(radians(parsedAngle)), -(height - height * 0.12) * sin(radians(parsedAngle)));
  popMatrix();
}
 
void displayText() {
  pushMatrix();
  statusMessage = (parsedDistance > 40) ? "Out of Range" : "In Range";
  fill(0, 0, 0);
  noStroke();
  rect(0, height - height * 0.0648, width, height);
  fill(98, 245, 31);
  textSize(25);
  text("10cm", width - width * 0.3854, height - height * 0.0833);
  text("20cm", width - width * 0.281, height - height * 0.0833);
  text("30cm", width - width * 0.177, height - height * 0.0833);
  text("40cm", width - width * 0.0729, height - height * 0.0833);
  textSize(40);
  text("SciCraft", width - width * 0.875, height - height * 0.0277);
  text("Angle: " + parsedAngle + "°", width - width * 0.48, height - height * 0.0277);
  text("Distance: ", width - width * 0.26, height - height * 0.0277);
  if (parsedDistance < 40) {
    text("               " + parsedDistance + " cm", width - width * 0.225, height - height * 0.0277);
  }
  popMatrix();
}
 