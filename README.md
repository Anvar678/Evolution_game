# Game of Life GA

Упрощённая версия “Жизни” с элементами генетики на C++ и WinAPI. Каждая клетка имеет набор из 64 “генов” (числа 1–10), которые определяют её поведение: движение, защита, атака, сбор энергии, поедание трупов, деление и размножение.

## Как собрать и запустить

1. Скопируйте файл `main.cpp` в папку проекта.  
2. Откройте любой C++ проект под Windows (например, в Visual Studio или MinGW).  
3. Добавьте `main.cpp` в проект и убедитесь, что указана поддержка C++11 (или выше).  
4. Постройте (Build) и запустите (Run) получившийся `.exe`.  
5. Откроется окно размером ~1500×800 пикселей (150×80 клеток по 10 px). Симуляция стартует сразу.

## Основные параметры (в начале `main.cpp`)

```cpp
// Параметры вывода на экран (в пикселях)
const int SIZE_X = 150;
const int SIZE_Y = 80;
const int SIZE_OF_SQ = 10;
const int TIME_OF_TACT = 100;
const int TIMER_ID = 1;

// Параметры клетки (которая живая)
const int START_POPULATION = 100; //стартовая популяция
const int MAX_CELLS = START_POPULATION * 100000; //максимум уникальных клеток
const int START_ENERGY = 100; //стартовая энергия
const int ENERGY_FOR_LIFE = 1; //энергия за 1 такт (просто за жизнь)
const int ENERGY_FOR_STEP = 5; //энергия за шаг движения
const int ENERGY_FROM_BODY = 10; // энергия от поедания трупа
const int ENERGY_FOR_SEX = 20;  //минимальная энергия которая должна быть у обоих родитлелей чтобы размножиться
const int ENERGY_FOR_SPLIT = 30; //минимальная энергия которая нужна для деления 
const int SPLIT_LESS_ENERGY = 50; //процент энергии, которую родитель теряет при делении (отдает сыну)
const int RADIUS_OF_SEX = 2; // радиус в котором родители могут заниматься кхм...
const int SIMILAR_PERCENT_OF_GENS = 80; //процент схожести генов чтобы считаться одним видом и краситься в 1 цвет
const int SIMILAR_PERCENT_OF_GENS_FOR_SEX = 20; //нужный процент совпадения генов чтобы родители могли размножаться половым путем
const int SEX_LESS_ENERGY = 35; //сколько энергии детё берет от родаков в процентах (от каждого по столько %)
const int ENERGY_FIRST_LAYER = 40; //энергия от солнца на первом(верхнем слое)
const int ENERGY_SECOND_LAYER = 30; //энергия от солнца на среднем слое
const int ENERGY_THIRD_LAYER = 20; //энергия на 3-м слое
const int NUMBER_OF_SIGNS = 64; // число тактов в 1 цикле т.е. число генов в клетке
const int NUMBER_OF_GENES = 10;  // виды генов
const int MUTATIONS_PER_OFFSPRING = 5; //сколько генов мутируют при размножении
const int TIME_OF_CORPSE = 192; // время лежания трупа до того как он исчезнет
const bool BEAUTIFUL_COLORS = false; //цвета не будут дуюлироваться у разных видов, но будут лагать

const bool CHANCE_GENERATION = true; //включить измененную вероятность изначальной генерации генрв (придется дольше ждать в начале)
const int CHANCE_OF_GEN_1 = 10; //10- обычная вероятность, чем ниже число, тем ниже шанс и наоборот
const int CHANCE_OF_GEN_2 = 10;
const int CHANCE_OF_GEN_3 = 10;
const int CHANCE_OF_GEN_4 = 10;
const int CHANCE_OF_GEN_5 = 10;
const int CHANCE_OF_GEN_6 = 10;
const int CHANCE_OF_GEN_7 = 20;
const int CHANCE_OF_GEN_8 = 10;
const int CHANCE_OF_GEN_9 = 10;
const int CHANCE_OF_GEN_10 = 10;
