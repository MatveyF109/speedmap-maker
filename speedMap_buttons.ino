#define BTN1_PIN 2 // Кнопка 1
uint32_t BTN1_lastTime = 0;
uint32_t BTN1_cnt = 0;
uint32_t BTN1_totalCnt = 0;

#define BTN2_PIN 3 // Кнопка 2
bool BTN2_state = 1;
uint32_t BTN2_lastTime = 0;
uint32_t BTN2_cnt = 0;
uint32_t BTN2_totalCnt = 0;

uint32_t spdCalc_lastTime = 0;                 // для проверки что едем по прямой
int spdCalc_counter = 0;                       // кол-во прямых отрезков по 50 мсек подряд

struct spdMap {
  String mode = "nothing";
  uint32_t roundsNum = 0;
};
spdMap spdMap_element[16];                     // изначально записываем значения сюда
int elementsCounter = 0;                       // чтобы знать в какой элемент записывать значения
spdMap P_spdMap_element[16];                   // P=parsed после очистки от повторений значения пишем сюда
String P_mode;
String P_prev_mode;
int sameModeElems_counter = 1;
int elemsRounds_summ = 0;                      // сумма оборотов всех подряд идущих элементов с одинаковым mode
int P_position = 0;                                // место для записи в паренной структуре

bool state = 1;                                // led state

bool flag1 = false;
bool flag2 = false;

void btn1() {                                  //функция для опроса кнопки в прерывании
  if (millis() - BTN1_lastTime > 200) {
    digitalWrite(LED_BUILTIN, state);
    state = !state;
    BTN1_cnt++;
    BTN1_totalCnt++;
    BTN1_lastTime = millis();
  }
}

void btn2() {                                 //функция для опроса кнопки в прерывании
  if (millis() - BTN2_lastTime > 200) {
    digitalWrite(LED_BUILTIN, state);
    state = !state;
    BTN2_cnt++;
    BTN2_totalCnt++;
    BTN2_lastTime = millis();
  }
}

void setup() {
  Serial.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);

  attachInterrupt(0, btn1, RISING);
  attachInterrupt(1, btn2, RISING);
}


void loop() {
  while (millis() < 30000) {
    //  Serial.print(BTN1_cnt);
    //  Serial.print("   ");
    //  Serial.println(BTN2_cnt);
    if ((millis() - spdCalc_lastTime) > 1000) {       //смотрим итоги каждого "шага" 50 мсек !!!!!!!!!!!!!!!
      ////////////////////////////////////////////////////////
      int delta = BTN1_cnt - BTN2_cnt;
      if (2 >= delta) flag1 = true;
      if (delta >= -2) flag2 = true;
      ////////////////////////////////////////////////////////
      if (flag1 && flag2) {
        //      Serial.print("on straight line");
        //      Serial.print("   ");
        //      Serial.print(BTN1_cnt);
        //      Serial.print("   ");
        //      Serial.println(BTN2_cnt);
        spdCalc_counter++;
        BTN1_cnt = 0;
        BTN2_cnt = 0;
      }
      else {
        //      Serial.print("on turn");
        //      Serial.print("   ");
        //      Serial.print(BTN1_cnt);
        //      Serial.print("   ");
        //      Serial.println(BTN2_cnt);
        if (spdCalc_counter > 5) { //запомнить время 20 шагов со скоростью f            !!!!!!!!!!!!!!
          spdMap_element[elementsCounter].mode = 'f';
          spdMap_element[elementsCounter].roundsNum = (BTN1_totalCnt + BTN2_totalCnt) / 2;
          Serial.print(spdMap_element[elementsCounter].roundsNum);
          Serial.print(":");
          Serial.print(spdMap_element[elementsCounter].mode);
          Serial.print(" ");
          elementsCounter++;

        }
        else {
          spdMap_element[elementsCounter].mode = 'n';
          spdMap_element[elementsCounter].roundsNum = (BTN1_totalCnt + BTN2_totalCnt) / 2;
          Serial.print(spdMap_element[elementsCounter].roundsNum);
          Serial.print(":");
          Serial.print(spdMap_element[elementsCounter].mode);
          Serial.print(" ");
          elementsCounter++;
        }
        spdCalc_counter = 0; //обнуляем т. к. прямая кончилась и нужно начать считать время и расстояние без поворотов сначала
        BTN1_totalCnt = 0;
        BTN2_totalCnt = 0;
      }
      BTN1_cnt = 0;
      BTN2_cnt = 0;
      spdCalc_lastTime = millis();
      flag1 = false;
      flag2 = false;

    }
  }
  //////////////////////////////////////////////////////////////////// удаление повторений  pos 2 elems 3
  for (int i = 0; i < 15; i++) {
    P_prev_mode = spdMap_element[i].mode;
    P_mode = spdMap_element[i + 1].mode;
    if (P_prev_mode == P_mode) {
      int pos = i;         // чтобы знать откуда начать перебор элементов с одинаковым mode
      sameModeElems_counter = 1;   // перед каждум подсчетом надо обнулить, но для правильной работы нужно на 1 элемент больше
      while (P_prev_mode == P_mode) {
        sameModeElems_counter++;
        pos++;
        P_prev_mode = spdMap_element[pos].mode;
        P_mode = spdMap_element[pos + 1].mode;
      }
      elemsRounds_summ = 0;
      for (int a = i; a < (i + sameModeElems_counter); a++) {
        elemsRounds_summ += spdMap_element[a].roundsNum;
      }
      P_spdMap_element[P_position].mode = spdMap_element[i].mode;
      P_spdMap_element[P_position].roundsNum = elemsRounds_summ;
      P_position++;
      i = i + sameModeElems_counter - 1;      // все эти элементы слиты в 1
    }

    else {
      P_spdMap_element[P_position].mode = spdMap_element[i].mode;
      P_spdMap_element[P_position].roundsNum = spdMap_element[i].roundsNum;
      P_position++;
    }
  }
  Serial.println("");
  for (int i = 0; i < 16; i++) {
    //if (P_spdMap_element[i].mode != "nothing") {
      Serial.print(P_spdMap_element[i].mode);
      Serial.print(":");
      Serial.print(P_spdMap_element[i].roundsNum);
      Serial.print("   ");
    //}
  }
  delay(60000);


}
