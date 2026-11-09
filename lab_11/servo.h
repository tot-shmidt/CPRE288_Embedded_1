//In main.c these variables have to be setup before the call to servo init, as the variables are used in it.
extern unsigned int zero_deg_match_val;
extern unsigned int oneEighty_deg_match_val;

void servo_init();

int servo_move(float degrees);
