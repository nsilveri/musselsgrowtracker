#ifndef BMA400_H
#define BMA400_H


class BMA400Functions {
public:
  void  begin();
  void  readAcceleration(float *destination);
  void  setOffset(float *destination);
  float convertToMetersPerSecondSquared(int16_t rawValue);
  int16_t readTempData();
  //void  myinthandler1();
  //void  myinthandler2();
private:
  uint8_t _intPin1;
  uint8_t _intPin2;
};

extern BMA400Functions BMA400Func;

#endif  // BMA400_H