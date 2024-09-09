const int ButtonUP = 12;
const int ButtonDOWN = 11;
const int ButtonGreen = 8;
const int ButtonRed = 9;

const int Relay1 = 6;
const int Relay2 = 10;

bool pushed_up = true;
bool pushed_down = true;
bool pushed_green = true;
bool pushed_red = true;

void setup (void)
{
  pinMode (ButtonUP, INPUT_PULLUP);
  pinMode (ButtonDOWN, INPUT_PULLUP);
  pinMode (ButtonGreen, INPUT_PULLUP);
  pinMode (ButtonRed, INPUT_PULLUP);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
}

void loop (void)
{
  bool prev_up = pushed_up;
  bool prev_down = pushed_down;
  bool prev_green = pushed_green;
  bool prev_red = pushed_red;

  pushed_up = digitalRead (ButtonUP) == LOW;
  pushed_down = digitalRead (ButtonDOWN) == LOW;
  pushed_green = digitalRead (ButtonGreen) == LOW;
  pushed_red = digitalRead (ButtonRed) == LOW;

 if (prev_up != pushed_up)
    {
      if (pushed_up && !pushed_down) // Prevents multiple button pushed
        {
          digitalWrite(Relay1, LOW);
        }
      else                                                          
        {
          digitalWrite(Relay1, HIGH);
        }
    }

 if (prev_down != pushed_down)
    {
      if (pushed_down && !pushed_up) // Prevents multiple button pushed                               
        {
          digitalWrite(Relay2, LOW);
        }
      else                                                      
        {
          digitalWrite(Relay2, HIGH);
        }
    }

 if (prev_green != pushed_green)
    {
      if (pushed_green)   
        {
          for(int i=0; i<5; i++) {
            digitalWrite(Relay1, LOW);
            delay(200);
            digitalWrite(Relay1, HIGH);
            delay(200);
          }
        }
      else  
        {
          digitalWrite(Relay1, HIGH);
        }
    }

   if (prev_red != pushed_red)
    {
      if (pushed_red)    
        {
          for(int i=0; i<5; i++) {
            digitalWrite(Relay2, LOW);
            delay(200);
            digitalWrite(Relay2, HIGH);
            delay(200);
          }
        }
      else  
        {
          digitalWrite(Relay2, HIGH);
        }
    }
}