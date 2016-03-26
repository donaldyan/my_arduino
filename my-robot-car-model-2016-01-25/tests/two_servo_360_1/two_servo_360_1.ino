#define N 20
int servoPinOne = 9;
int servoPinTwo = 10;

int servoPosition = 1000; // position in microseconds

void setup() 
{
  pinMode(servoPinOne, OUTPUT);
  pinMode(servoPinTwo, OUTPUT);
  Serial.begin(9600); 
}

void loop() {
  Forward();
  Forward();
  Forward();
  delay(2000);

  Backward();
  Backward();
  Backward();
  delay(2000);

//  RightRotate();
//  delay(2000);
  
//  LeftRotate();
//  delay(2000);
}

void Forward() {
  int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinOne, LOW);
    
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void Backward() {
  int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinOne, LOW);
    
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void RightRotate() { // vpravo
   int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    digitalWrite(servoPinOne, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void LeftRotate() { // vlavo
    int i = 0;
    while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(1000); // toci sa vlavo
    digitalWrite(servoPinOne, LOW);
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  } 
}
