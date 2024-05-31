# Fungames

![изображение](https://github.com/mavexe/Fungames/assets/82767270/34d1b0c3-a08e-4794-bd4d-67a32734e5cc)

Here's the stats of our players

## Math & Explanation

We have C(n,k) = n!*(n-k)! (Where's n=8 k=4) methods of divide 8 people in 2 groups stricly(e.c. 50/50) C(8,4)=8!/(4!*4!)=70
But! VItalya does not wants to join into the Marat group -> We need to cut all of the cominations where Marat and Vitalya in one group
In this case math problem -> C(6,3) = 6!/3!*(6-3)! = 20 <-- 
2 Type -> C(6,4) = 6!/4!*(6-4)!=15*2(Marat & Vitalya can switch group) => 30 => 30+20==50!

But we have the "Weights" of the players so we need to combine all logic including the weights.

# Players
![изображение](https://github.com/mavexe/Fungames/assets/82767270/187a7312-fd30-4f1e-a392-2cf5919fd683)
