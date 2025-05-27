#include <Servo.h>

Servo moteurH;
Servo moteurV;

int position_horizontal = 90;
int position_vertical = 140;

void setup() {
  Serial.begin(9600);
  moteurH.attach(6);
  moteurV.attach(5);
}

void loop() {
  if (Serial.available()) {
    String donnees = Serial.readStringUntil('\n'); 

    int horizontal = donnees.indexOf('h');
    int vertical = donnees.indexOf('v');

    if (horizontal != -1 && vertical != -1) {
      String horizontal_string = donnees.substring(0, horizontal);
      String vertical_string = donnees.substring(horizontal + 1, vertical);

      int new_pos_horizontal = horizontal_string.toInt();
      int new_pos_vertical = vertical_string.toInt();

      new_pos_horizontal = constrain(new_pos_horizontal, 0, 180);
      new_pos_vertical = constrain(new_pos_vertical, 0, 180);

      moteurH.write(new_pos_horizontal);
      moteurV.write(new_pos_vertical);

      position_horizontal = new_pos_horizontal;
      position_vertical = new_pos_vertical;
    }
  }
}
