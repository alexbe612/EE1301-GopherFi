GopherFi by Alex Beck (beck1951@umn.edu), Hamse Salex (salex005@umn.edu), Adam Adam (adam2072@umn.edu).

The purpose of this project was to create an interactive map on an html webpage displaying the Wifi signal strength as recorded points around UMN campus. 
The used internet connection for this project had SSID: UofM-IoT

A photon 2 was used for this project, and the particle project folder used was /EE1301-GopherFi/GopherFi/. The code flashed to the photon can be found in 
/GopherFi/src/GopherFi.cpp. Once the program is successfully flashed to the photon, the usb may be unplugged from the photon and replaced with a 5V power supply, 
connecting to VUSB and GND as the positive and negative terminals respectively. The photon itself has an internal resistor that converts 5V into 3.3V, so 
the 3.3V source will function normally. 

When the button is pressed, an iLED lights up blue for a second to indicate the button has been pressed. Then the GPS sends latitude and longitude data over to the
photon, and the photon recordes the current signal strength it recieves (on a 0-100 scale). These 3 variables are then published as an event to particle.io.

Once the event is published, a Google Sheet reads the published data and records it. In order to do this, an app script was used within Google Sheets.
To open the script, navigate to Extensions > App Script. A code.gs file should be opened. Replace the current code in code.gs with the map_spreadsheet.js code. 
Then press Deploy > New Deployment, and allow access to everyone, so the script can read the particle event. For more information on how to connect particle to 
Google Sheets, visit [this link.](https://docs.particle.io/integrations/community-integrations/publish-to-google-sheets/). 

In the particle console, navigate to Cloud Services > Integrations > Google Sheets. Make sure the request type is POST and the format is Web Form.
Note: The event name MUST be the same name listed in GopherFi.cpp Line 36 (We used "sheet-data"). This is to trigger the webhook successfully. 
The URL in the webhook builder can be found within the App Script under Deploy > Manage Deployments > URL. Copy and paste this URL to the webhook.

Once the spreadsheet is setup, go to Share > Publish to Web on Google Sheets. The second option should be reading "Web Page".
Change this to "Comma-Separated Values (.csv)". Copy the link displayed and paste it into "index.html" as the variable "sheetCsvUrl" (line 113).

To view the webpage, we used GitHub Pages. Simply opening the html file will not connect to the spreadsheet, because certain security issues occur without 
proper deployment. Another option to view the html page is through cloudfare pages. 

To add data points to the spreadsheet, press the button on the device. To plot these data points on the map, refresh the html page. To remove data points, 
delete the corresponding row in the spreadsheet.



