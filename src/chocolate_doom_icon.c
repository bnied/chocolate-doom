static int chocolate_doom_w = 32;
static int chocolate_doom_h = 32;

static unsigned char chocolate_doom_data[] = {
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  119,79,43,  
    127,83,47,  155,99,59,  155,99,59,  127,83,47,  119,79,43,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    119,79,43,  135,87,51,  143,95,55,  143,95,55,  143,95,55,  
    143,95,55,  135,87,51,  119,79,43,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  119,79,43,  143,95,55,  135,87,51,  135,87,51,  
    135,87,51,  135,87,51,  135,87,51,  135,87,51,  143,95,55,  
    119,79,43,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  119,79,43,  179,115,71,  
    155,99,59,  127,83,47,  107,71,39,  107,71,39,  127,83,47,  
    155,99,59,  179,115,71,  143,95,55,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  127,83,47,  
    171,111,67,  191,123,75,  179,115,71,  163,107,63,  135,87,51,  
    135,87,51,  163,107,63,  179,115,71,  191,123,75,  171,111,67,  
    127,83,47,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  135,87,51,  95,67,35,  75,55,27,  135,87,51,  
    155,99,59,  171,111,67,  171,111,67,  155,99,59,  135,87,51,  
    75,55,27,  95,67,35,  135,87,51,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  135,87,51,  107,71,39,  
    115,0,0,  255,31,31,  51,43,19,  63,47,23,  75,55,27,  
    51,43,19,  255,31,31,  115,0,0,  107,71,39,  135,87,51,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  135,87,51,  107,71,39,  79,0,0,  75,55,27,  
    107,71,39,  107,71,39,  75,55,27,  79,0,0,  107,71,39,  
    135,87,51,  107,71,39,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    135,87,51,  143,95,55,  143,95,55,  119,79,43,  135,87,51,  
    95,67,35,  107,71,39,  143,95,55,  143,95,55,  107,71,39,  
    95,67,35,  135,87,51,  119,79,43,  95,67,35,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  119,119,119,  131,131,131,  0,0,0,  
    171,171,171,  99,99,99,  0,0,0,  0,0,0,  127,83,47,  
    135,87,51,  143,95,55,  143,95,55,  135,87,51,  119,79,43,  
    95,67,35,  135,87,51,  43,35,15,  131,131,131,  255,255,255,  
    179,179,179,  131,131,131,  43,35,15,  135,87,51,  75,55,27,  
    51,43,19,  75,55,27,  127,83,47,  171,171,171,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  119,119,119,  127,127,127,  
    0,0,0,  0,0,0,  119,119,119,  223,223,223,  127,127,127,  
    127,83,47,  143,95,55,  163,107,63,  155,99,59,  135,87,51,  
    95,67,35,  75,55,27,  63,47,23,  127,83,47,  43,35,15,  
    43,35,15,  43,35,15,  43,35,15,  43,35,15,  43,35,15,  
    127,83,47,  63,47,23,  75,55,27,  95,67,35,  143,95,55,  
    135,87,51,  155,99,59,  135,87,51,  119,119,119,  159,159,159,  
    127,127,127,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    119,119,119,  107,71,39,  143,95,55,  163,107,63,  171,111,67,  
    135,87,51,  95,67,35,  63,47,23,  51,43,19,  51,43,19,  
    119,79,43,  43,35,15,  127,0,0,  43,35,15,  43,35,15,  
    127,0,0,  43,35,15,  119,79,43,  51,43,19,  75,55,27,  
    95,67,35,  163,107,63,  143,95,55,  127,83,47,  223,223,223,  
    159,159,159,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  107,71,39,  107,71,39,  107,71,39,  127,83,47,  
    135,87,51,  163,107,63,  155,99,59,  143,95,55,  95,67,35,  
    75,55,27,  51,43,19,  95,67,35,  43,35,15,  127,0,0,  
    103,0,0,  103,0,0,  155,0,0,  43,35,15,  95,67,35,  
    51,43,19,  95,67,35,  119,79,43,  155,99,59,  135,87,51,  
    119,79,43,  127,127,127,  127,83,47,  107,71,39,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  107,71,39,  83,63,31,  
    83,63,31,  107,71,39,  127,83,47,  135,87,51,  135,87,51,  
    127,83,47,  127,83,47,  95,67,35,  75,55,27,  75,55,27,  
    95,67,35,  127,0,0,  103,0,0,  103,0,0,  155,0,0,  
    95,67,35,  75,55,27,  51,43,19,  95,67,35,  143,95,55,  
    135,87,51,  127,83,47,  119,79,43,  95,67,35,  119,79,43,  
    95,67,35,  95,67,35,  0,0,0,  0,0,0,  0,0,0,  
    107,71,39,  63,47,23,  75,55,27,  63,47,23,  75,55,27,  
    107,71,39,  127,83,47,  127,83,47,  119,79,43,  107,71,39,  
    75,55,27,  63,47,23,  75,55,27,  127,0,0,  103,0,0,  
    127,0,0,  127,0,0,  75,55,27,  51,43,19,  83,63,31,  
    119,79,43,  119,79,43,  119,79,43,  107,71,39,  95,67,35,  
    51,43,19,  63,47,23,  75,55,27,  83,63,31,  0,0,0,  
    0,0,0,  0,0,0,  83,63,31,  83,63,31,  51,43,19,  
    51,43,19,  63,47,23,  75,55,27,  83,63,31,  131,131,131,  
    183,183,183,  95,67,35,  95,67,35,  63,47,23,  63,47,23,  
    155,0,0,  103,0,0,  191,167,143,  191,167,143,  191,167,143,  
    63,47,23,  95,67,35,  131,131,131,  107,71,39,  83,63,31,  
    63,47,23,  63,47,23,  63,47,23,  75,55,27,  131,131,131,  
    191,191,191,  203,203,203,  171,171,171,  0,0,0,  83,63,31,  
    75,55,27,  51,43,19,  51,43,19,  43,35,15,  43,35,15,  
    51,43,19,  63,47,23,  63,47,23,  75,55,27,  83,63,31,  
    75,55,27,  63,47,23,  179,0,0,  103,0,0,  127,0,0,  
    167,143,119,  191,167,143,  167,143,119,  191,167,143,  63,47,23,  
    83,63,31,  63,47,23,  51,43,19,  51,43,19,  75,55,27,  
    83,63,31,  99,99,99,  99,99,99,  99,99,99,  0,0,0,  
    0,0,0,  75,55,27,  63,47,23,  51,43,19,  43,35,15,  
    43,35,15,  51,43,19,  51,43,19,  51,43,19,  63,47,23,  
    75,55,27,  83,63,31,  83,63,31,  95,67,35,  63,47,23,  
    155,0,0,  127,0,0,  167,143,119,  191,167,143,  191,167,143,  
    167,143,119,  63,47,23,  63,47,23,  51,43,19,  43,35,15,  
    63,47,23,  75,55,27,  95,67,35,  107,71,39,  95,67,35,  
    75,55,27,  0,0,0,  0,0,0,  63,47,23,  51,43,19,  
    43,35,15,  43,35,15,  43,35,15,  63,47,23,  63,47,23,  
    63,47,23,  43,35,15,  51,43,19,  63,47,23,  75,55,27,  
    83,63,31,  63,47,23,  51,43,19,  63,47,23,  147,123,99,  
    167,143,119,  191,167,143,  167,143,119,  255,207,179,  51,43,19,  
    43,35,15,  51,43,19,  63,47,23,  75,55,27,  107,71,39,  
    119,79,43,  95,67,35,  75,55,27,  0,0,0,  0,0,0,  
    51,43,19,  51,43,19,  43,35,15,  43,35,15,  43,35,15,  
    63,47,23,  63,47,23,  43,35,15,  43,35,15,  51,43,19,  
    51,43,19,  51,43,19,  63,47,23,  51,43,19,  63,47,23,  
    75,55,27,  147,123,99,  147,123,99,  147,123,99,  147,123,99,  
    167,143,119,  255,187,147,  63,47,23,  51,43,19,  51,43,19,  
    63,47,23,  95,67,35,  107,71,39,  83,63,31,  75,55,27,  
    0,0,0,  0,0,0,  51,43,19,  43,35,15,  43,35,15,  
    43,35,15,  63,47,23,  43,35,15,  63,47,23,  51,43,19,  
    63,47,23,  75,55,27,  83,63,31,  95,67,35,  83,63,31,  
    75,55,27,  63,47,23,  63,47,23,  255,207,179,  167,143,119,  
    147,123,99,  167,143,119,  255,207,179,  239,163,115,  63,47,23,  
    43,35,15,  43,35,15,  51,43,19,  63,47,23,  75,55,27,  
    83,63,31,  75,55,27,  0,0,0,  0,0,0,  51,43,19,  
    51,43,19,  43,35,15,  63,47,23,  0,0,0,  0,0,0,  
    51,43,19,  63,47,23,  75,55,27,  83,63,31,  75,55,27,  
    63,47,23,  75,55,27,  83,63,31,  75,55,27,  43,35,15,  
    255,187,147,  255,187,147,  255,207,179,  255,207,179,  239,163,115,  
    107,71,39,  135,87,51,  95,67,35,  95,67,35,  51,43,19,  
    75,55,27,  83,63,31,  107,71,39,  107,71,39,  83,63,31,  
    0,0,0,  43,35,15,  43,35,15,  63,47,23,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  63,47,23,  63,47,23,  
    63,47,23,  63,47,23,  63,47,23,  75,55,27,  63,47,23,  
    51,43,19,  43,35,15,  63,47,23,  239,163,115,  239,163,115,  
    239,163,115,  107,71,39,  95,67,35,  135,87,51,  135,87,51,  
    107,71,39,  95,67,35,  107,71,39,  135,87,51,  155,99,59,  
    155,99,59,  135,87,51,  83,63,31,  63,47,23,  83,63,31,  
    75,55,27,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  63,47,23,  51,43,19,  51,43,19,  51,43,19,  
    51,43,19,  51,43,19,  63,47,23,  63,47,23,  63,47,23,  
    51,43,19,  51,43,19,  179,115,71,  107,71,39,  95,67,35,  
    135,87,51,  155,99,59,  135,87,51,  135,87,51,  95,67,35,  
    135,87,51,  135,87,51,  135,87,51,  127,83,47,  83,63,31,  
    95,67,35,  83,63,31,  75,55,27,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  63,47,23,  51,43,19,  
    51,43,19,  63,47,23,  83,63,31,  107,71,39,  83,63,31,  
    51,43,19,  95,67,35,  107,71,39,  75,55,27,  51,43,19,  
    43,35,15,  239,163,115,  43,35,15,  95,67,35,  95,67,35,  
    107,71,39,  95,67,35,  107,71,39,  107,71,39,  107,71,39,  
    95,67,35,  83,63,31,  83,63,31,  63,47,23,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    83,63,31,  63,47,23,  51,43,19,  51,43,19,  75,55,27,  
    75,55,27,  63,47,23,  51,43,19,  63,47,23,  75,55,27,  
    75,55,27,  51,43,19,  51,43,19,  179,115,71,  239,163,115,  
    63,47,23,  95,67,35,  95,67,35,  95,67,35,  95,67,35,  
    83,63,31,  75,55,27,  75,55,27,  0,0,0,  63,47,23,  
    63,47,23,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  83,63,31,  95,67,35,  83,63,31,  
    63,47,23,  63,47,23,  63,47,23,  63,47,23,  63,47,23,  
    63,47,23,  63,47,23,  51,43,19,  51,43,19,  51,43,19,  
    63,47,23,  43,35,15,  63,47,23,  43,35,15,  75,55,27,  
    83,63,31,  75,55,27,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  51,43,19,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  75,55,27,  
    83,63,31,  95,67,35,  95,67,35,  95,67,35,  107,71,39,  
    95,67,35,  107,71,39,  119,79,43,  119,79,43,  95,67,35,  
    75,55,27,  51,43,19,  63,47,23,  83,63,31,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  51,43,19,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    63,47,23,  75,55,27,  75,55,27,  83,63,31,  83,63,31,  
    95,67,35,  107,71,39,  83,63,31,  63,47,23,  95,67,35,  
    107,71,39,  83,63,31,  63,47,23,  75,55,27,  75,55,27,  
    83,63,31,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    51,43,19,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  63,47,23,  75,55,27,  75,55,27,  
    75,55,27,  75,55,27,  83,63,31,  83,63,31,  75,55,27,  
    95,67,35,  95,67,35,  83,63,31,  63,47,23,  75,55,27,  
    83,63,31,  95,67,35,  75,55,27,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  51,43,19,  51,43,19,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  63,47,23,  
    75,55,27,  75,55,27,  75,55,27,  75,55,27,  75,55,27,  
    75,55,27,  75,55,27,  107,71,39,  107,71,39,  107,71,39,  
    107,71,39,  75,55,27,  75,55,27,  75,55,27,  75,55,27,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  
    0,0,0,  0,0,0,  0,0,0,  0,0,0,  
};
