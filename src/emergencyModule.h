#ifndef _EMERGENCYMODULE_H_
#define _EMERGENCYMODULE_H_

struct GPSData {
  char UTCtime[10];
  double latitude;
  char latitudeDir;
  double longitude;
  char longitudeDir;
  int quality;
  int numSats;
  float altitude;
  float bearing;
  bool valid;
};

#endif
