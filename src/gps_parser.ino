struct GPSData 
getPos()
{
  char NMEA[88];
  char GPRMC[88];
  struct GPSData tmpPos = {" ", 0.0, 'F', 0.0, 'F', 0, 0, 0.0, 0.0, false};
  int index = 0;
  char work[12];
  char *comp;

  // Grab GPGGA message containing most relavant position data
  if (Serial1.find("$GPGGA,")){
    index = Serial1.readBytesUntil(0x0D, NMEA, 88);
    
    // Uncomment to see raw serial string
    //Serial.println(NMEA);
  }

  // Grab GPRMC message containing the bearing in true north format
  if (Serial1.find("$GPRMC,")){
    Serial1.readBytesUntil(0x0D, GPRMC, 88);

    // Uncomment to see raw serial string
    //Serial.println(GPRMC);
  }

  if (index > 60 ){
    comp = strtok(NMEA, ",");

    // Store UTC time in struct
    strcpy(tmpPos.UTCtime, comp);

    // Move on to latitude
    comp = strtok(NULL, ","); 
    strcpy(work, comp); 
    tmpPos.latitude = degreeConversion(work);

    // Store N or S in direction
    comp = strtok(NULL, ","); 
    tmpPos.latitudeDir = *comp; 

    // Move on to longitude
    comp = strtok(NULL, ","); 
    strcpy(work, comp); 
    tmpPos.longitude = degreeConversion(work);  

    // Store E or W in direction
    comp = strtok(NULL, ","); 
    tmpPos.longitudeDir = *comp;

    // Store message quality
    comp = strtok(NULL, ",");
    tmpPos.quality = atoi(comp);

    // Store number of satellites
    comp = strtok(NULL, ",");
    tmpPos.numSats= atoi(comp);

    // Skip to mean sea level
    strtok(NULL, ",");
    strtok(NULL, ",");
    strtok(NULL, ",");

    // Store mean sea level
    comp = strtok(NULL, ",");
    tmpPos.altitude = (float) atof(comp);   
  }

  if (index > 60 ){
    comp = strtok(GPRMC, ",");

    //Skip information until bearing
    for(int i = 0; i < 7; i++){
      comp = strtok(NULL, ",");
    }

      (atof(comp) > 0.0 && atof(comp) < 360.0) ? tmpPos.bearing = atof(comp): tmpPos.bearing = gps_data.bearing;
  } 

  // Check if position should be considered valid
  if(tmpPos.quality >= 1 && tmpPos.numSats >= 3){
    tmpPos.valid = true;
  }else{
    tmpPos.valid = false;
  }

  return tmpPos;  
}


double 
degreeConversion(char* nmea)
{
  double conversion = 0.0;
  double deg = 0.0;
  double decimals = 0.0;
  int p = 1;
  int i = 0;
  
  // Skip starting 0 in longitude
  if (strlen(nmea) > 10){
    i++;
  }
  
  // Conversion from hours, mins format into degrees
  for(i ;i < strlen(nmea); i++){
  
    if(nmea[i] == '.'){
      continue;
  }
  
    conversion = (float)(nmea[i] - '0');
  
    if (p >= 0){
      deg += (conversion * pow(10, p));
    }else{
      decimals += (conversion * pow(10, p + 2));
    }
  
    p--;
  }
  
  decimals = decimals / 60.0;
  deg += decimals;
  
  return deg;
}


bool 
newCoordinates(double longitude, 
               double latitude)
{
    bool newValue = false;
    double diff = 0.0;
    float coordinateOffset = 0.000005;

    diff = gps_data.longitude - longitude;

    if (diff < 0.0){
      diff = diff * -1.0;
    }

    if (diff > coordinateOffset){
      newValue = true;
    }

    diff = gps_data.latitude - latitude;

    if (diff < 0.0){
      diff = diff * -1.0;
    }
  
    if (diff > coordinateOffset){
      newValue = true;
    }

    return newValue;
}
