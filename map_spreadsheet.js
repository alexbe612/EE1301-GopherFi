function doPost(e) {
  
  // Sets the output sheet to the current one, and gets the data from the particle event as a JSON
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var rawData = e.parameter.data; 

  try {
    
    // Splits the JSON data into 3 values based on latitude, longitude, and signal strength
    var sensorValues = JSON.parse(rawData); 
    
    // Creates a row with 4 columns: The date, and the 3 values obtained from the JSON (particle event)
    var newRow = [new Date()].concat(sensorValues);
    
    // Places the data into the next blank row
    sheet.appendRow(newRow);
    
    // The return values only show withn the script editor and were used for debugging
    return ContentService.createTextOutput("Success");
    
  } catch(error) {
    
    return ContentService.createTextOutput("Error parsing data: " + error.toString());
  }
}