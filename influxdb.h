#ifndef INFLUXDB_H
#define INFLUXDB_H

void initInfluxDB();
void closeInfluxDB();
void writeToInfluxDB(const char *measurement, const char *payload);

#endif
