state_puts (s, state)
     char *s;
     state_t *state;
{
  puts (s, state->fpout);
