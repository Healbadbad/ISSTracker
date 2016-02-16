from sgp4.earth_gravity import wgs72
from sgp4.io import twoline2rv
import sgp4
import time
import ephem
from math import pi
from serial import Serial
try:
  s = Serial('COM30', 9600)
except:
  print "Pic Not opened!"

def main():
  line0 = 'ISS (ZARYA)'
  line1 = '1 25544U 98067A   16045.08461514  .00015278  00000-0  23253-3 0  9996'
  line2 = '2 25544  51.6435  316.9119 0006811 112.2536  34.0677 15.5475799985676'

  tracker = ephem.Observer()
  tracker.lat = '39.16532'
  tracker.long = '-86.52639'

  target = 0
  targetAz = 0
  targetElev = 0

  tracker.elevation = 152
  # tracker.date = str(currentTime.tm_year) + "/" + str(currentTime.tm_mon) + "/" + str(currentTime.tm_mday) + " " + str(currentTime.tm_hour + 1) +":"+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec)

  print "TRACKER INFORMATION --------------"
  print tracker.date, tracker.lat, tracker.long


  print "ISS INFORMATION -----------------------"
  iss= ephem.readtle(line0, line1, line2)
  iss.compute()
  print iss.sublat, iss.sublong

  sun = ephem.Sun()
  sun.compute()

  moon = ephem.Moon()
  moon.compute()


  while True:
    # now =  time.time()
    # currentTime =  time.localtime()
    # tracker.date = str(currentTime.tm_year) + "/" + str(currentTime.tm_mon) + "/" + str(currentTime.tm_mday) + " " + str(currentTime.tm_hour) +":"+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec)

    tracker = ephem.Observer()
    tracker.lat = '39.16532'
    tracker.long = '-86.52639'
    tracker.elevation = 152

    if target ==0:
      print "ISS RELATIVE TO TRACKER ---------------"
      iss.compute(tracker)
      print iss.sublat, iss.sublong
      print iss.alt, iss.az
      targetAz = iss.az
      targetElev = iss.alt

    if target == 1:
      print "Sun RELATIVE TO TRACKER ---------------"
      sun.compute(tracker)
      # print sun.sublat, sun.sublong
      print sun.alt, sun.az
      targetAz = sun.az
      targetElev = sun.alt

    if target == 2:
      print "Moon RELATIVE TO TRACKER ---------------"
      moon.compute(tracker)
      # print sun.sublat, sun.sublong
      print moon.alt, moon.az
      targetAz = moon.az
      targetElev = moon.alt

    if target == 3:
      print "RESET RELATIVE TO TRACKER ---------------"
      # sun.compute(tracker)
      # print sun.sublat, sun.sublong
      print 0, 0
      targetAz = 0
      targetElev = 0
    
    print "bits ready: ", s.inWaiting()
    if s.inWaiting():
      target = int(s.read(1))
      print target
    print target

    sendToPic(targetAz, targetElev)

    
    # time.sleep(1)


def sendToPic(azm, elev):
  ''' Send elevation and azimuth to pic for moving the motors '''

  print "motor steps to be at: " + str(int(((azm*180/pi) * (3.832/1.282) /1.8)))

  print "servo number: " + str(int( (elev*180/pi +90) *19/18 + 50))
  az = int(((azm*180/pi) * (3.832/1.282) /1.8))
  alt = int( (elev*180/pi +90) *19/18 + 50)
  # send each character to pic - like lab
  print str(int(az/100)) + str(int(az%100)/10) + str(int(az%10)) 
  print str(int(alt/100)) + str(int(alt%100)/10) + str(int(alt%10)) 
  s.write('A')
  s.write(str(int(az/100)) + str(int(az%100)/10) + str(int(az%10)) )
  time.sleep(1)
  s.write('E')
  s.write(str(int(alt/100)) + str(int(alt%100)/10) + str(int(alt%10)) )
if __name__ == "__main__":
  main()