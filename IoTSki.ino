/*
The sketch demonstrates iBecaon from an RFduino
*/

/*
 Copyright (c) 2014 OpenSourceRF.com.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <RFduinoBLE.h>
#include <ADXL362.h>
#include <SPI.h>
#include <Timer.h>

Timer t;
ADXL362 xl;
int16_t XValue, YValue, ZValue, Temperature;
byte isConnected=false;
int led = 1;

void doEvery();
int doWakeup(uint32_t ulPin);

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  xl.begin();                //soft reset
  delay(1000);


  //  Setup Activity and Inactivity thresholds
  //     tweaking these values will effect the "responsiveness" and "delay" of the interrupt function
  //     my settings result in a very rapid, sensitive, on-off switch, with a 2 second delay to sleep when motion stops
  xl.setupDCActivityInterrupt(300, 10);		// 300 code activity threshold.  With default ODR = 100Hz, time threshold of 10 results in 0.1 second time threshold
  xl.setupDCInactivityInterrupt(80, 200);		// 80 code inactivity threshold.  With default ODR = 100Hz, time threshold of 30 results in 2 second time threshold
  Serial.println();

  /* Other possible settings
  //  Motion activated On - stays on for 60 seconds
  xl.setupDCActivityInterrupt(300, 10);		// 300 code activity threshold.  With default ODR = 100Hz, time threshold of 10 results in 0.1 second time threshold
  xl.setupDCInactivityInterrupt(80, 6000);	// 80 code inactivity threshold.  With default ODR = 100Hz, time threshold of 60000 results in 60 second time threshold
  */


  //
  // Setup ADXL362 for proper autosleep mode
  //

  // Map Awake status to Interrupt 1
  // *** create a function to map interrupts... coming soon
  xl.SPIwriteOneRegister(0x2A, 0x40);

  // Setup Activity/Inactivity register
  xl.SPIwriteOneRegister(0x27, 0x3F); // Referenced Activity, Referenced Inactivity, Loop Mode

  // turn on Autosleep bit
  byte POWER_CTL_reg = xl.SPIreadOneRegister(0x2D);
  POWER_CTL_reg = POWER_CTL_reg | (0x04);				// turn on POWER_CTL[2] - Autosleep bit
  xl.SPIwriteOneRegister(0x2D, POWER_CTL_reg);

  //
  // turn on Measure mode
  //
  xl.beginMeasure();                      // DO LAST! enable measurement mode
  xl.checkAllControlRegs();               // check some setup conditions
  delay(100);
  RFduinoBLE.advertisementInterval = 100;
//  RFduinoBLE.txPowerLevel = -20;
  // start the BLE stack
  pinMode(2, INPUT);
  RFduino_pinWakeCallback(2, LOW, doWakeup);
  RFduinoBLE.begin();
  t.every(20000, doEvery);
}

void loop() {
  t.update();
}

void RFduinoBLE_onConnect()
{
  isConnected = true;
}

void RFduinoBLE_onDisconnect()
{
  isConnected = false;
}

void RFduinoBLE_onAdvertisement(bool start)
{
}

void doEvery()
{
  if (!isConnected)
  {
    Serial.print("sleep");
    Serial.println();
    delay(1000);
    isConnected==false;
    RFduino_systemOff();
  }
}

int doWakeup(uint32_t ulPin)
{
    Serial.print("wakeup");
    Serial.println();
    t.every(20000, doEvery);
  }



