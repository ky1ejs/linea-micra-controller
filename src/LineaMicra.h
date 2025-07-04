
#ifndef LINEA_MICRA_H
#define LINEA_MICRA_H

class LineaMicra
{
public:
  LineaMicra();

private:
  bool isOn;
  float temperature;

  bool preBrewIsOn;
  float preBrewTime;
  float preBrewWait;
};

#endif // LINEA_MICRA_H