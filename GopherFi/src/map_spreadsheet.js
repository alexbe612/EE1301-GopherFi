function doPost(e) {
  // Get the active sheet
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  // Particle sends the string payload in a parameter called "data"
  var rawData = e.parameter.data; 

  try {
    // Because your format [x,y,z] is a valid JSON array, we can parse it instantly!
    var sensorValues = JSON.parse(rawData); 
    
    // Create a new row starting with the current date/time, followed by your 3 values
    var newRow = [new Date()].concat(sensorValues);
    
    // Append the row to the bottom of the sheet
    sheet.appendRow(newRow);
    
    // Return a success message
    return ContentService.createTextOutput("Success");
    
  } catch(error) {
    // If something goes wrong (e.g., bad formatting), log it
    return ContentService.createTextOutput("Error parsing data: " + error.toString());
  }
}