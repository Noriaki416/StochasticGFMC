  weight[0] = 0.0;
  weight[1] = 0.5;
  weight[2] = 0.5;
  weight[3] = 0.0;
  weight[4] = 0.0;
  weight[5] = 0.0;
  weight[6] = 0.0;
  // vkind: 1 inleg: 0 outleg: 0 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 1 inleg: 0 outleg: 1 vkind_after_flipp: 4
  // 0 1 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 1 inleg: 0 outleg: 2 vkind_after_flipp: 5
  // 0 1 -> 1 1
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 1 inleg: 0 outleg: 3 vkind_after_flipp: 0
  // 0 1 -> 1 1
  // 0 1 -> 0 0    weight: 0.0
  vkind_after_flipped1[1][0] = 4;
  leg_after_flipped1[1][0] = 1;
  vkind_after_flipped2[1][0] = 1;
  leg_after_flipped2[1][0] = 0;
  vkind_after_flipped3[1][0] = 5;
  leg_after_flipped3[1][0] = 2;
  prob1[1][0] = 0.5/(0.5+0.5+0.0);
  prob2[1][0] = 1.0;
  // vkind: 1 inleg: 1 outleg: 0 vkind_after_flipp: 4
  // 0 1 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 1 inleg: 1 outleg: 1 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 1 inleg: 1 outleg: 2 vkind_after_flipp: 0
  // 0 1 -> 0 0
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 1 inleg: 1 outleg: 3 vkind_after_flipp: 6
  // 0 1 -> 0 0
  // 0 1 -> 0 0    weight: 0.0
  vkind_after_flipped1[1][1] = 4;
  leg_after_flipped1[1][1] = 0;
  vkind_after_flipped2[1][1] = 1;
  leg_after_flipped2[1][1] = 1;
  vkind_after_flipped3[1][1] = 6;
  leg_after_flipped3[1][1] = 3;
  prob1[1][1] = 0.5/(0.5+0.5+0.0);
  prob2[1][1] = 1.0;
  // vkind: 1 inleg: 2 outleg: 0 vkind_after_flipp: 5
  // 0 1 -> 1 1
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 1 inleg: 2 outleg: 1 vkind_after_flipp: 0
  // 0 1 -> 0 0
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 1 inleg: 2 outleg: 2 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 1 inleg: 2 outleg: 3 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 0 1 -> 1 0    weight: 0.5
  vkind_after_flipped1[1][2] = 3;
  leg_after_flipped1[1][2] = 3;
  vkind_after_flipped2[1][2] = 1;
  leg_after_flipped2[1][2] = 2;
  vkind_after_flipped3[1][2] = 5;
  leg_after_flipped3[1][2] = 0;
  prob1[1][2] = 0.5/(0.5+0.5+0.0);
  prob2[1][2] = 1.0;
  // vkind: 1 inleg: 3 outleg: 0 vkind_after_flipp: 0
  // 0 1 -> 1 1
  // 0 1 -> 0 0    weight: 0.0
  // vkind: 1 inleg: 3 outleg: 1 vkind_after_flipp: 6
  // 0 1 -> 0 0
  // 0 1 -> 0 0    weight: 0.0
  // vkind: 1 inleg: 3 outleg: 2 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 0 1 -> 1 0    weight: 0.5
  // vkind: 1 inleg: 3 outleg: 3 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  vkind_after_flipped1[1][3] = 3;
  leg_after_flipped1[1][3] = 2;
  vkind_after_flipped2[1][3] = 1;
  leg_after_flipped2[1][3] = 3;
  vkind_after_flipped3[1][3] = 6;
  leg_after_flipped3[1][3] = 1;
  prob1[1][3] = 0.5/(0.5+0.5+0.0);
  prob2[1][3] = 1.0;
  // vkind: 2 inleg: 0 outleg: 0 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 2 inleg: 0 outleg: 1 vkind_after_flipp: 3
  // 1 0 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 2 inleg: 0 outleg: 2 vkind_after_flipp: 6
  // 1 0 -> 0 0
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 2 inleg: 0 outleg: 3 vkind_after_flipp: 0
  // 1 0 -> 0 0
  // 1 0 -> 1 1    weight: 0.0
  vkind_after_flipped1[2][0] = 3;
  leg_after_flipped1[2][0] = 1;
  vkind_after_flipped2[2][0] = 2;
  leg_after_flipped2[2][0] = 0;
  vkind_after_flipped3[2][0] = 6;
  leg_after_flipped3[2][0] = 2;
  prob1[2][0] = 0.5/(0.5+0.5+0.0);
  prob2[2][0] = 1.0;
  // vkind: 2 inleg: 1 outleg: 0 vkind_after_flipp: 3
  // 1 0 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 2 inleg: 1 outleg: 1 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 2 inleg: 1 outleg: 2 vkind_after_flipp: 0
  // 1 0 -> 1 1
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 2 inleg: 1 outleg: 3 vkind_after_flipp: 5
  // 1 0 -> 1 1
  // 1 0 -> 1 1    weight: 0.0
  vkind_after_flipped1[2][1] = 3;
  leg_after_flipped1[2][1] = 0;
  vkind_after_flipped2[2][1] = 2;
  leg_after_flipped2[2][1] = 1;
  vkind_after_flipped3[2][1] = 5;
  leg_after_flipped3[2][1] = 3;
  prob1[2][1] = 0.5/(0.5+0.5+0.0);
  prob2[2][1] = 1.0;
  // vkind: 2 inleg: 2 outleg: 0 vkind_after_flipp: 6
  // 1 0 -> 0 0
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 2 inleg: 2 outleg: 1 vkind_after_flipp: 0
  // 1 0 -> 1 1
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 2 inleg: 2 outleg: 2 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 2 inleg: 2 outleg: 3 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 1 0 -> 0 1    weight: 0.5
  vkind_after_flipped1[2][2] = 4;
  leg_after_flipped1[2][2] = 3;
  vkind_after_flipped2[2][2] = 2;
  leg_after_flipped2[2][2] = 2;
  vkind_after_flipped3[2][2] = 6;
  leg_after_flipped3[2][2] = 0;
  prob1[2][2] = 0.5/(0.5+0.5+0.0);
  prob2[2][2] = 1.0;
  // vkind: 2 inleg: 3 outleg: 0 vkind_after_flipp: 0
  // 1 0 -> 0 0
  // 1 0 -> 1 1    weight: 0.0
  // vkind: 2 inleg: 3 outleg: 1 vkind_after_flipp: 5
  // 1 0 -> 1 1
  // 1 0 -> 1 1    weight: 0.0
  // vkind: 2 inleg: 3 outleg: 2 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 1 0 -> 0 1    weight: 0.5
  // vkind: 2 inleg: 3 outleg: 3 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  vkind_after_flipped1[2][3] = 4;
  leg_after_flipped1[2][3] = 2;
  vkind_after_flipped2[2][3] = 2;
  leg_after_flipped2[2][3] = 3;
  vkind_after_flipped3[2][3] = 5;
  leg_after_flipped3[2][3] = 1;
  prob1[2][3] = 0.5/(0.5+0.5+0.0);
  prob2[2][3] = 1.0;
  // vkind: 3 inleg: 0 outleg: 0 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 3 inleg: 0 outleg: 1 vkind_after_flipp: 2
  // 0 1 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 3 inleg: 0 outleg: 2 vkind_after_flipp: 0
  // 0 1 -> 1 1
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 3 inleg: 0 outleg: 3 vkind_after_flipp: 5
  // 0 1 -> 1 1
  // 1 0 -> 1 1    weight: 0.0
  vkind_after_flipped1[3][0] = 3;
  leg_after_flipped1[3][0] = 0;
  vkind_after_flipped2[3][0] = 2;
  leg_after_flipped2[3][0] = 1;
  vkind_after_flipped3[3][0] = 5;
  leg_after_flipped3[3][0] = 3;
  prob1[3][0] = 0.5/(0.5+0.5+0.0);
  prob2[3][0] = 1.0;
  // vkind: 3 inleg: 1 outleg: 0 vkind_after_flipp: 2
  // 0 1 -> 1 0
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 3 inleg: 1 outleg: 1 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 3 inleg: 1 outleg: 2 vkind_after_flipp: 6
  // 0 1 -> 0 0
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 3 inleg: 1 outleg: 3 vkind_after_flipp: 0
  // 0 1 -> 0 0
  // 1 0 -> 1 1    weight: 0.0
  vkind_after_flipped1[3][1] = 3;
  leg_after_flipped1[3][1] = 1;
  vkind_after_flipped2[3][1] = 2;
  leg_after_flipped2[3][1] = 0;
  vkind_after_flipped3[3][1] = 6;
  leg_after_flipped3[3][1] = 2;
  prob1[3][1] = 0.5/(0.5+0.5+0.0);
  prob2[3][1] = 1.0;
  // vkind: 3 inleg: 2 outleg: 0 vkind_after_flipp: 0
  // 0 1 -> 1 1
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 3 inleg: 2 outleg: 1 vkind_after_flipp: 6
  // 0 1 -> 0 0
  // 1 0 -> 0 0    weight: 0.0
  // vkind: 3 inleg: 2 outleg: 2 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  // vkind: 3 inleg: 2 outleg: 3 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 1 0 -> 0 1    weight: 0.5
  vkind_after_flipped1[3][2] = 3;
  leg_after_flipped1[3][2] = 2;
  vkind_after_flipped2[3][2] = 1;
  leg_after_flipped2[3][2] = 3;
  vkind_after_flipped3[3][2] = 6;
  leg_after_flipped3[3][2] = 1;
  prob1[3][2] = 0.5/(0.5+0.5+0.0);
  prob2[3][2] = 1.0;
  // vkind: 3 inleg: 3 outleg: 0 vkind_after_flipp: 5
  // 0 1 -> 1 1
  // 1 0 -> 1 1    weight: 0.0
  // vkind: 3 inleg: 3 outleg: 1 vkind_after_flipp: 0
  // 0 1 -> 0 0
  // 1 0 -> 1 1    weight: 0.0
  // vkind: 3 inleg: 3 outleg: 2 vkind_after_flipp: 1
  // 0 1 -> 0 1
  // 1 0 -> 0 1    weight: 0.5
  // vkind: 3 inleg: 3 outleg: 3 vkind_after_flipp: 3
  // 0 1 -> 0 1
  // 1 0 -> 1 0    weight: 0.5
  vkind_after_flipped1[3][3] = 3;
  leg_after_flipped1[3][3] = 3;
  vkind_after_flipped2[3][3] = 1;
  leg_after_flipped2[3][3] = 2;
  vkind_after_flipped3[3][3] = 5;
  leg_after_flipped3[3][3] = 0;
  prob1[3][3] = 0.5/(0.5+0.5+0.0);
  prob2[3][3] = 1.0;
  // vkind: 4 inleg: 0 outleg: 0 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 4 inleg: 0 outleg: 1 vkind_after_flipp: 1
  // 1 0 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 4 inleg: 0 outleg: 2 vkind_after_flipp: 0
  // 1 0 -> 0 0
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 4 inleg: 0 outleg: 3 vkind_after_flipp: 6
  // 1 0 -> 0 0
  // 0 1 -> 0 0    weight: 0.0
  vkind_after_flipped1[4][0] = 4;
  leg_after_flipped1[4][0] = 0;
  vkind_after_flipped2[4][0] = 1;
  leg_after_flipped2[4][0] = 1;
  vkind_after_flipped3[4][0] = 6;
  leg_after_flipped3[4][0] = 3;
  prob1[4][0] = 0.5/(0.5+0.5+0.0);
  prob2[4][0] = 1.0;
  // vkind: 4 inleg: 1 outleg: 0 vkind_after_flipp: 1
  // 1 0 -> 0 1
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 4 inleg: 1 outleg: 1 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 4 inleg: 1 outleg: 2 vkind_after_flipp: 5
  // 1 0 -> 1 1
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 4 inleg: 1 outleg: 3 vkind_after_flipp: 0
  // 1 0 -> 1 1
  // 0 1 -> 0 0    weight: 0.0
  vkind_after_flipped1[4][1] = 4;
  leg_after_flipped1[4][1] = 1;
  vkind_after_flipped2[4][1] = 1;
  leg_after_flipped2[4][1] = 0;
  vkind_after_flipped3[4][1] = 5;
  leg_after_flipped3[4][1] = 2;
  prob1[4][1] = 0.5/(0.5+0.5+0.0);
  prob2[4][1] = 1.0;
  // vkind: 4 inleg: 2 outleg: 0 vkind_after_flipp: 0
  // 1 0 -> 0 0
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 4 inleg: 2 outleg: 1 vkind_after_flipp: 5
  // 1 0 -> 1 1
  // 0 1 -> 1 1    weight: 0.0
  // vkind: 4 inleg: 2 outleg: 2 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  // vkind: 4 inleg: 2 outleg: 3 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 0 1 -> 1 0    weight: 0.5
  vkind_after_flipped1[4][2] = 4;
  leg_after_flipped1[4][2] = 2;
  vkind_after_flipped2[4][2] = 2;
  leg_after_flipped2[4][2] = 3;
  vkind_after_flipped3[4][2] = 5;
  leg_after_flipped3[4][2] = 1;
  prob1[4][2] = 0.5/(0.5+0.5+0.0);
  prob2[4][2] = 1.0;
  // vkind: 4 inleg: 3 outleg: 0 vkind_after_flipp: 6
  // 1 0 -> 0 0
  // 0 1 -> 0 0    weight: 0.0
  // vkind: 4 inleg: 3 outleg: 1 vkind_after_flipp: 0
  // 1 0 -> 1 1
  // 0 1 -> 0 0    weight: 0.0
  // vkind: 4 inleg: 3 outleg: 2 vkind_after_flipp: 2
  // 1 0 -> 1 0
  // 0 1 -> 1 0    weight: 0.5
  // vkind: 4 inleg: 3 outleg: 3 vkind_after_flipp: 4
  // 1 0 -> 1 0
  // 0 1 -> 0 1    weight: 0.5
  vkind_after_flipped1[4][3] = 4;
  leg_after_flipped1[4][3] = 3;
  vkind_after_flipped2[4][3] = 2;
  leg_after_flipped2[4][3] = 2;
  vkind_after_flipped3[4][3] = 6;
  leg_after_flipped3[4][3] = 0;
  prob1[4][3] = 0.5/(0.5+0.5+0.0);
  prob2[4][3] = 1.0;
  // vkind: 5 inleg: 0 outleg: 0 vkind_after_flipp: 5
  // 1 1 -> 1 1
  // 1 1 -> 1 1    weight: 0.0
  // vkind: 5 inleg: 0 outleg: 1 vkind_after_flipp: 0
  // 1 1 -> 0 0
  // 1 1 -> 1 1    weight: 0.0
  // vkind: 5 inleg: 0 outleg: 2 vkind_after_flipp: 1
  // 1 1 -> 0 1
  // 1 1 -> 0 1    weight: 0.5
  // vkind: 5 inleg: 0 outleg: 3 vkind_after_flipp: 3
  // 1 1 -> 0 1
  // 1 1 -> 1 0    weight: 0.5
  vkind_after_flipped1[5][0] = 3;
  leg_after_flipped1[5][0] = 3;
  vkind_after_flipped2[5][0] = 1;
  leg_after_flipped2[5][0] = 2;
  vkind_after_flipped3[5][0] = 5;
  leg_after_flipped3[5][0] = 0;
  prob1[5][0] = 0.5/(0.5+0.5+0.0);
  prob2[5][0] = 1.0;
  // vkind: 5 inleg: 1 outleg: 0 vkind_after_flipp: 0
  // 1 1 -> 0 0
  // 1 1 -> 1 1    weight: 0.0
  // vkind: 5 inleg: 1 outleg: 1 vkind_after_flipp: 5
  // 1 1 -> 1 1
  // 1 1 -> 1 1    weight: 0.0
  // vkind: 5 inleg: 1 outleg: 2 vkind_after_flipp: 4
  // 1 1 -> 1 0
  // 1 1 -> 0 1    weight: 0.5
  // vkind: 5 inleg: 1 outleg: 3 vkind_after_flipp: 2
  // 1 1 -> 1 0
  // 1 1 -> 1 0    weight: 0.5
  vkind_after_flipped1[5][1] = 4;
  leg_after_flipped1[5][1] = 2;
  vkind_after_flipped2[5][1] = 2;
  leg_after_flipped2[5][1] = 3;
  vkind_after_flipped3[5][1] = 5;
  leg_after_flipped3[5][1] = 1;
  prob1[5][1] = 0.5/(0.5+0.5+0.0);
  prob2[5][1] = 1.0;
  // vkind: 5 inleg: 2 outleg: 0 vkind_after_flipp: 1
  // 1 1 -> 0 1
  // 1 1 -> 0 1    weight: 0.5
  // vkind: 5 inleg: 2 outleg: 1 vkind_after_flipp: 4
  // 1 1 -> 1 0
  // 1 1 -> 0 1    weight: 0.5
  // vkind: 5 inleg: 2 outleg: 2 vkind_after_flipp: 5
  // 1 1 -> 1 1
  // 1 1 -> 1 1    weight: 0.0
  // vkind: 5 inleg: 2 outleg: 3 vkind_after_flipp: 0
  // 1 1 -> 1 1
  // 1 1 -> 0 0    weight: 0.0
  vkind_after_flipped1[5][2] = 4;
  leg_after_flipped1[5][2] = 1;
  vkind_after_flipped2[5][2] = 1;
  leg_after_flipped2[5][2] = 0;
  vkind_after_flipped3[5][2] = 5;
  leg_after_flipped3[5][2] = 2;
  prob1[5][2] = 0.5/(0.5+0.5+0.0);
  prob2[5][2] = 1.0;
  // vkind: 5 inleg: 3 outleg: 0 vkind_after_flipp: 3
  // 1 1 -> 0 1
  // 1 1 -> 1 0    weight: 0.5
  // vkind: 5 inleg: 3 outleg: 1 vkind_after_flipp: 2
  // 1 1 -> 1 0
  // 1 1 -> 1 0    weight: 0.5
  // vkind: 5 inleg: 3 outleg: 2 vkind_after_flipp: 0
  // 1 1 -> 1 1
  // 1 1 -> 0 0    weight: 0.0
  // vkind: 5 inleg: 3 outleg: 3 vkind_after_flipp: 5
  // 1 1 -> 1 1
  // 1 1 -> 1 1    weight: 0.0
  vkind_after_flipped1[5][3] = 3;
  leg_after_flipped1[5][3] = 0;
  vkind_after_flipped2[5][3] = 2;
  leg_after_flipped2[5][3] = 1;
  vkind_after_flipped3[5][3] = 5;
  leg_after_flipped3[5][3] = 3;
  prob1[5][3] = 0.5/(0.5+0.5+0.0);
  prob2[5][3] = 1.0;
