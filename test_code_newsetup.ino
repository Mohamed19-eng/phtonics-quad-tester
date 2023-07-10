#define down 3
#define up 4
#define right 5
#define left 6
volatile int incomingByte = 0 ;
int dir = 0;
int distancestep = 150;
int distancestep1 = 60;
int distancestep2 = 50; // um
int distance_update = 0;
int  remainder = 20; 
#include <ezButton.h>
#include <ADS1256.h>
#include <SPI.h>
#define __AVR_ATmega328P__
float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
// Initialize ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v
ezButton limitSwitch(7);  
// 25 pulses vertical stage equates to 10 um in half step mode 20 in full step mode
// when testing for the beam diameter it showed a distance covered of 1170 micrometer before the signal was completly lost from the QPD.
float sensor0,sensor1, sensor2, sensor3, sensor4;
class motor_command {       // The class
  public:             // Access specifier
    int number_of_steps;        // Attribute (int variable)
    int Direction;  // Attribute (string variable)
    void move()
      {
        for(int x = 0; x<number_of_pulses; x++)
            {
                Serial.println(Direction);
                delay(2);
                Serial.println("high");
                digitalWrite( direction ,HIGH);
                delayMicroseconds(100);
                digitalWrite(direction,LOW);
                delayMicroseconds(20);
                Serial.println("low");      
            }
      }
};
void setup() {
  pinMode(6,OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(3, OUTPUT);
  Serial.begin(9600);
  // put your setup code here, to run once:
  Serial.println("Starting ADC");
  adc.begin(ADS1256_DRATE_50SPS,ADS1256_GAIN_1,false); 
  limitSwitch.setDebounceTime(50);  
  Serial.println("ADC Started");
   adc.setChannel(0,1);
   // Set MUX Register to AINO and AIN1 so it start doing the ADC conversion
  //adc.setChannel(0,-1);
}
int position_calculation(int ch1 ,int ch2, int ch3, int ch4);
int compute_numOfpulses(int currentpos);
int moveforwardsHorizontal(int number_of_pulses);
int movebackwardsHorizontal(int number_of_pulses);
int autotest(int direction);
void loop() {
  motor_command move_command;
  move_command.number_of_steps = 10;
  move_command .Direction =   down;

 // limitSwitch.loop(); // MUST call the loop() function first
  limitSwitch.loop();
  int state = limitSwitch.getState();
 if(state ==  LOW)
  {
    move(left, 100);
    delay(5000);
  }

   //move(down, 1);
   //delay(5);
  /*
   move(up,500);
   delay(5);
   move(down, 500);
   delay(5);*/
  /*if(!(remainder < 15 && remainder > -15))
  {
     remainder = compute_numOfpulses(900);
  }*/
  //Serial.print("remain , num");
  //Serial.println(remain , num);
  if (Serial.available() > 0) {
     incomingByte = Serial.read();
      if((char)incomingByte == 'F')
      {
         movebackwardsHorizontal(1);
        log_adc_differential();
      }
     
       if((char)incomingByte == 'B')
      {
        moveforwardsHorizontal(1);
        //log_adc_differential();
      } 
     if((char)incomingByte == 'T')
       {
         while((char)incomingByte != 'B'){
          incomingByte = Serial.read();
          log_adc_differential();
         }
       }
       
      if((char)incomingByte == 'G')
       {

       }
    if((char)incomingByte == 'L'){
      intialize_test();
      Serial.println(" L");
    }
    if((char)incomingByte == 'P'){
      moveDown(750);
    }
    if((char)incomingByte == 'R'){
      autotest(5);
    }
     if((char)incomingByte == 'Q'){
     for(int i =0 ; i < 16;i++)
     {
         
         delay(3500);
         moveforwardsHorizontal(1);
     }
     for(int i =0 ; i < 16;i++)
     {
        delay(3500);
         movebackwardsHorizontal(1);
         
     }
     //movebackwardsVertical(15);
    }
   if((char)incomingByte == 'N'){
     int distance_covered = 0;
     // 1170 was the measured distance two times
     //Serial.println(" L");
     log_adc_single();
     //moveDown(1000);
     //distance_covered = beam_diameter();
      Serial.println(distance_covered);
     //movebackwardsVertical(15);
    }
  }   
}

int compute_numOfpulses(int currentpos)
{
  int remainder , number_of_steps = 0; 
   remainder = currentpos % distancestep;
   currentpos =  currentpos - remainder; 
   number_of_steps = currentpos * 2 / distancestep;
   delay(100);
   moveforwardsHorizontal(number_of_steps);
  if(remainder > 90)
    {
        number_of_steps++;
        moveforwardsHorizontal(1);
        remainder = remainder - distancestep1;
    }
while(1)
{
  if( number_of_steps % 2 != 0)
  {
    delay(1000);
    if(remainder > 40 && remainder < 60 )
    { 
      moveforwardsHorizontal(2);
      remainder = remainder - distancestep;
      number_of_steps = number_of_steps + 3;
      movebackwardsHorizontal(1);
      remainder = remainder + distancestep2;
    }
    else{
       moveforwardsHorizontal(1);
      remainder = remainder - distancestep2;
       number_of_steps++;
    }
    delay(10000);
    if(remainder <= 40 && remainder > 10)
    {
      
      movebackwardsHorizontal(1);
      remainder = remainder + distancestep2;
      moveforwardsHorizontal(1);
      remainder = remainder - distancestep1;
      number_of_steps = number_of_steps + 2;
    }
  }
  else{
    delay(1000);
    if(remainder > 40 )
    { 
      remainder = remainder - distancestep1;
      moveforwardsHorizontal(1);
      number_of_steps++;
    }
    else if(remainder < 40 && remainder > 10)
    {
      moveforwardsHorizontal(1);
      remainder = remainder + distancestep1;
      movebackwardsHorizontal(1);
      remainder = remainder + distancestep2;
      number_of_steps = number_of_steps + 2;
    }
  }
   //Serial.println(number_of_steps); 

  if( abs(remainder) == 10 || remainder == 0)
    {   
        Serial.println(remainder);
        return remainder; 
    }
 
   /*if(remainder % distancestep1 == 0 && number_of_steps % 2 == 0 ){
        number_of_steps++;
        remainder = remainder - distancestep1;
        Serial.println("remainder more than 60 and next is 60"); 
     }
  if(distancestep1 - abs(remainder) == 10 && number_of_steps % 2 == 0)
    {
      number_of_steps++;
      if(remainder < 0)
      {
        movebackwardsHorizontal(1);
        remainder = remainder + distancestep1;    
      }
      return number_of_steps;
    }
     
     //add retun;
    if(remainder % distancestep2 == 0 && number_of_steps % 2 != 0 ){
        number_of_steps++;
        remainder = remainder - distancestep2;
        Serial.println("remainder more than 90 and next is 90"); 
      }
    if(distancestep2 - abs(remainder) == 10 && number_of_steps % 2 != 0)
      {
        number_of_steps++;
        if(remainder < 0)
        {
          movebackwardsHorizontal(1);
            remainder = remainder + distancestep2;    
        }
      return number_of_steps;
      }
   else{
     if(number_of_steps % 2 == 0)
     {
       remainder = remainder - distancestep1;
       number_of_steps++;
     }
     else
     {
       remainder = remainder - distancestep2;
       number_of_steps++;
     }
    
     /*if(number_of_steps % 2 == 0 && distancestep1 - remainder < 15 )
     {
       number_of_steps++;
       Serial.println("remainder less than 60 or 90 and next is 60");
       break;
     }
     else if(number_of_steps % 2 != 0 && distancestep2 - remainder < 15)
     {
       number_of_steps++;
       Serial.println("remainder less than 60 or 90 and next is 90");
       break;
     }
     else
     {
       Serial.println("remainder less than 60 or 90 and next is 90 or 60 but the difference is more than 10");
      number_of_steps++;
      movebackwardsHorizontal(2);
      break;
     }
    
   }*/
}
    //Serial.println("r- , n-");
   // Serial.println(remainder);
   Serial.println(number_of_steps);
}
  // put your main code here, to run repeatedly:
int moveforwardsHorizontal(int number_of_pulses)
{
  for(int x = 0; x<number_of_pulses; x++)
         {
            delay(2);
            digitalWrite(5,HIGH);
            delayMicroseconds(100);
            digitalWrite(5,LOW);
            delayMicroseconds(20);      
         }
}
int movebackwardsHorizontal(int number_of_pulses)
{
    for(int x = 0; x<number_of_pulses; x++)
         {
            delay(2);
            digitalWrite(6,HIGH);
            delayMicroseconds(100);
            digitalWrite(6,LOW);
            delayMicroseconds(20);      
         }
}
int moveDown(int number_of_pulses)
{
  for(int x = 0; x<number_of_pulses; x++)
         {
            delay(2);
            digitalWrite(3,HIGH);
            delayMicroseconds(100);
            digitalWrite(3,LOW);
            delayMicroseconds(20);      
         }
}
int moveUP(int number_of_pulses)
{
    for(int x = 0; x<number_of_pulses; x++)
         {
            delayMicroseconds(500);
            digitalWrite(4,HIGH);
            delayMicroseconds(100);
            digitalWrite(4,LOW);
            delayMicroseconds(20);      
         }
}

int autotest2(int direction)
{
  delay(10);
// movebackwardsHorizontal(8);
  int duration  = 0;
  int previousdir = direction;
  int even = 2;
  int distance_hor_positive = 0;
  int distance_hor_negative = 0;
  int distance_ver = 0;
  int check = 0;
         do{
           if(duration > 30 && previousdir != 6){ 
              previousdir = 6;
              duration = 0;
              //moveUP(25);
             // Serial.print(distance_ver);
              //Serial.println("\t");
              //distance_ver += 10;
            }
            else if(duration > 30 && previousdir != 5){ 
              previousdir = 5;
              duration = 0;
              moveUP(25);
              Serial.print("-");
              Serial.println(distance_ver);
              distance_ver += 10;
              
            }
            check = log_adc_differential();
            //log_adc_single();
            incomingByte = Serial.read();
            if((char)incomingByte == 'G')
              {
                break;
              }
           /* movebackwardsHorizontal(50);
            delay(2);
            moveUP(25);
            delay(2);
            moveforwardsHorizontal(50);
            delay(2);
            moveUP(25);
            delay(2);*/
            digitalWrite(previousdir,HIGH);
            delayMicroseconds(100);
            digitalWrite(previousdir,LOW);
            delayMicroseconds(5);
            duration++;
            delay(50);   
         }while((char)incomingByte != 'S');
         Serial.println("finished");
         return 0;
}
int autotest(int direction)
{
  int duration  = 0;
  int distance = 0;
  int stepcount =0;
  int previousdir = direction;
  int check = 0;
         do{
           //fast_scan();
           
           check = log_adc_differential();
            incomingByte = Serial.read();
            if((char)incomingByte == 'G')
              {
                break;
              }
            if(check == 1)
              {
               break;
              }
            if(duration > 150 && previousdir != 6){ 
              previousdir = 6;
              duration = 0;
              moveUP(250);
              stepcount=0;
            }
            else if(duration > 150 && previousdir != 5){ 
              previousdir = 5;
              duration = 0;
              moveUP(250);
              stepcount=0;
            }
            digitalWrite(previousdir,HIGH);
            delayMicroseconds(100);
            digitalWrite(previousdir,LOW);
            delayMicroseconds(50);
            duration++;
            stepcount++;
            delay(50);
         }while((char)incomingByte != 'S');
         Serial.println("finished");
         autotest2(6);
         return 0;

}
int beam_diameter()
{
  int micrometers = 0;
  int check = 0;
 
    while (check != 2)
    { 
        incomingByte = Serial.read();
        if((char)incomingByte == 'S')
        {
          break;
        }
        check = log_adc_differential();
        moveUP(25);
        delay(1500);
        micrometers += 10;
        Serial.println(micrometers);
    }
return micrometers;
}
int position_calculation(int ch1 ,int ch2, int ch3, int ch4){
  int x_pos = 0;
  int y_pos = 0;
  x_pos = ((ch1+ch2)-(ch3+ch4))/(ch1+ch2+ch3+ch4);
  y_pos = ((ch3+ch2)-(ch1+ch4))/(ch1+ch2+ch3+ch4);
  
}
void intialize_test()
{
  int distance = 0;
  bool check = 0;
  moveforwardsHorizontal(500);
  Serial.println("Reference point reached, scanning for QPD");
  if (Serial.available() > 0) {
     incomingByte = Serial.read();
  }
  delay(100);
  movebackwardsHorizontal(400);
  delay(100);
  while(1){
      //compute_numOfpulses(300);
      movebackwardsHorizontal(2);
      delay(150);
      check = log_adc_differential();
      if(check == 1 && check !=2)
      {
          break;
      }
      incomingByte = Serial.read();
      if((char)incomingByte == 'G')
        {
          break;
        }
  }
  Serial.println("QPD found");
  //autotest(6);
}
void log_adc_single()
{
  for(int i = 0; i<1000; i++)
  {
          adc.waitDRDY();
          //Serial.print(sensor1,10);
          adc.setChannel(0,-1);// switch back to MUX AIN0 
          sensor0 = adc.readCurrentChannel();
          adc.waitDRDY();
          //Serial.print(sensor1,10);
          adc.setChannel(1,-1);// switch back to MUX AIN1
          sensor1 = adc.readCurrentChannel();
          adc.waitDRDY();
          adc.setChannel(2,-1);// switch back to MUX AIN2
          sensor2 = adc.readCurrentChannel();
          Serial.print(sensor0,10);
          Serial.print("\t");
          Serial.print(sensor1,10);
          Serial.print("\t");
          Serial.print(sensor2,10);
          Serial.println("\t");
           incomingByte = Serial.read();
            if((char)incomingByte == 'G')
              {
                break;
              }
  }
}
bool log_adc_differential()
{
  float difference = 0; 
          adc.waitDRDY();
          // wait for DRDY to go low before changing multiplexer register
          adc.setChannel(2,3);   // Set the MUX for differential between ch2 and 3 
          sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and AIN1
          adc.waitDRDY();
          adc.setChannel(4,5);
          sensor2 = adc.readCurrentChannel(); //// DOUT arriving here are from MUX AIN2 and AIN3
          adc.waitDRDY();
          adc.setChannel(6,7);
          sensor3 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN4 and AIN5
          adc.waitDRDY();
          adc.setChannel(0,1); // switch back to MUX AIN0 and AIN1
          sensor4 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN6 and AIN7            
          //print the result.
          delayMicroseconds(500);   
  Serial.print(sensor1,10);
  Serial.print(",");
  Serial.print(sensor3,10);
  Serial.print(",");
  //Serial.print("\t");
//Serial.print(sensor3,10);
  Serial.println(sensor4,10);

  //abs(sensor1)> 0.045 &&
  if(abs(sensor3) > 0.072)
      {
        return 1;
      }
  else{
    return 0;
  }

}
void fast_scan()
{
  int check = 0;
            movebackwardsHorizontal(250);
            delay(50);
            moveUP(500);
            delay(2);
            moveforwardsHorizontal(250);
            delay(50);
            moveUP(500);
            delay(2);
            check = log_adc_differential();
            //log_adc_single();
             if(check == 1)
            {
              //delay(200);
              //moveforwardsHorizontal(8);
              autotest2(6);
            }
}
/*void slow_scan()
{
  int check = 0;
             if(duration > 150 && previousdir != 6){ 
              previousdir = 6;
              duration = 0;
              moveUP(500);

            }
            else if(duration > 150 && previousdir != 5){ 
              previousdir = 5;
              duration = 0;
              moveUP(500);
            }
            digitalWrite(previousdir,HIGH);
            delayMicroseconds(100);
            digitalWrite(previousdir,LOW);
            delayMicroseconds(50);
            duration++;
            delay(5);
}*/

