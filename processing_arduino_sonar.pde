import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial port; // Serial port object for communication with external hardware
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
  // Set up the display window and initialize serial communication
  //size(1200, 700); // Adjust to screen size
  size(2000, 1000);
  smooth();
  port = new Serial(this, "COM10", 9600); // Set serial port and baud rate
  port.bufferUntil('.'); // Read data until a '.' character
}

void draw() {
  // Main draw function to update visuals each frame
  fill(98, 245, 31); // Set fill color
  noStroke(); // No border on shapes
  fill(0, 4); // Semi-transparent background for motion blur effect
  rect(0, 0, width, height - height * 0.065); // Slight fade in the top area
  fill(98, 245, 31); // Reset color for radar elements

  drawRadarDisplay(); // Draw the radar arcs and angle lines
  drawDirectionLine(); // Draw the line indicating current angle
  drawDetectedObject(); // Draw the object based on angle and distance
  displayText(); // Display additional data text on screen
}

void serialEvent(Serial port) {
  // Handles data reading and parsing from the serial port
  rawData = port.readStringUntil('.'); // Read data until the delimiter '.'
  rawData = rawData.substring(4, rawData.length() - 1); // Remove trailing '.'
  
  
  commaIndex1 = rawData.indexOf(","); // Find the position of ',' to separate angle and distance
  print("rawData: ");
  println(rawData);
  print("commaIndex1: ");
  println(commaIndex1);
  angleData = rawData.substring(0, commaIndex1); // Extract angle data
  distanceData = rawData.substring(commaIndex1 + 1, rawData.length()); // Extract distance data
  
  parsedAngle = int(angleData); // Convert angle data to integer
  parsedDistance = int(distanceData); // Convert distance data to integer
  
  print(parsedAngle);
  print(",");
  println(parsedDistance);

}

void drawRadarDisplay() {
  // Draws radar arcs and angle guide lines
  pushMatrix();
  translate(width / 2, height - height * 0.074); // Set radar center position
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31); // Set color for radar lines
  
  // Draw concentric arcs representing distance zones
  arc(0, 0, width - width * 0.0625, width - width * 0.0625, PI, TWO_PI);
  arc(0, 0, width - width * 0.27, width - width * 0.27, PI, TWO_PI);
  arc(0, 0, width - width * 0.479, width - width * 0.479, PI, TWO_PI);
  arc(0, 0, width - width * 0.687, width - width * 0.687, PI, TWO_PI);

  // Draw angle guide lines every 30 degrees from 30° to 150°
  line(-width / 2, 0, width / 2, 0);
  for (int angle = 30; angle <= 150; angle += 30) {
    line(0, 0, (-width / 2) * cos(radians(angle)), (-width / 2) * sin(radians(angle)));
  }
  popMatrix();
}

void drawDetectedObject() {
  // Draws the detected object based on angle and distance
  pushMatrix();
  translate(width / 2, height - height * 0.074); // Set radar center position
  strokeWeight(9);
  stroke(255, 10, 10); // Red color for detected object
  distancePixels = parsedDistance * ((height - height * 0.1666) * 0.025); // Convert distance to pixels
  
  if (parsedDistance < 40) { // Limit display range to 40 cm
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
  // Draws a line showing the current angle on the radar
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60); // Green line color
  translate(width / 2, height - height * 0.074); // Set radar center position
  line(0, 0, (height - height * 0.12) * cos(radians(parsedAngle)), -(height - height * 0.12) * sin(radians(parsedAngle)));
  popMatrix();
}

void displayText() {
  // Display distance and angle information on screen
  pushMatrix();
  statusMessage = (parsedDistance > 40) ? "Out of Range" : "In Range"; // Set range message
  fill(0, 0, 0);
  noStroke();
  rect(0, height - height * 0.0648, width, height); // Display background for text
  fill(98, 245, 31); // Text color
  textSize(25);

  // Display distance markers
  text("10cm", width - width * 0.3854, height - height * 0.0833);
  text("20cm", width - width * 0.281, height - height * 0.0833);
  text("30cm", width - width * 0.177, height - height * 0.0833);
  text("40cm", width - width * 0.0729, height - height * 0.0833);
  
  textSize(40);
  text("SciCraft", width - width * 0.875, height - height * 0.0277); // Display title
  text("Angle: " + parsedAngle + "°", width - width * 0.48, height - height * 0.0277); // Display angle
  text("Distance: ", width - width * 0.26, height - height * 0.0277); // Display distance
  if (parsedDistance < 40) { // Display only within range
    text("               " + parsedDistance + " cm", width - width * 0.225, height - height * 0.0277);
  }
  
  textSize(25);
  fill(98, 245, 60); // Color for angle labels

  // Display angle markers from 30° to 150°
  float[] angles = {30, 60, 90, 120, 150};
  float[] angleTranslations = {0.4994, 0.503, 0.507, 0.513, 0.5104};
  int[] rotations = {-60, -30, 0, -30, -60};
  
  for (int i = 0; i < angles.length; i++) {
    resetMatrix();
    translate(
      (width - width * angleTranslations[i]) + width / 2 * cos(radians(angles[i])),
      (height - height * 0.0907) - width / 2 * sin(radians(angles[i]))
    );
    rotate(radians(rotations[i])); // Adjust rotation for each angle label
    text((int) angles[i] + "°", 0, 0); // Display angle label
  }
  popMatrix();
}
