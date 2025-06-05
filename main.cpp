#include <windows.h>
#include <vector>
#include <cstdlib> //rand от 0 до 32767
#include <ctime>
#include <string>

using namespace std;

//*----------------------------------------------------------------------------------------------------------------------------
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
const int MUTATIONS_PER_OFFSPRING = 0; //сколько генов мутируют при размножении
const int TIME_OF_CORPSE = 192; // время лежания трупа до того как он исчезнет
const bool BEAUTIFUL_COLORS = false; //цвета не будут дуюлироваться у разных видов, но будут лагать

const bool CHANCE_GENERATION = true; //включить измененную вероятность изначальной генерации генрв (придется дольше ждать в начале)
const int CHANCE_OF_GEN_1 = 0; //10- обычная вероятность, чем ниже число, тем ниже шанс и наоборот
const int CHANCE_OF_GEN_2 = 0;
const int CHANCE_OF_GEN_3 = 0;
const int CHANCE_OF_GEN_4 = 0;
const int CHANCE_OF_GEN_5 = 10;
const int CHANCE_OF_GEN_6 = 10;
const int CHANCE_OF_GEN_7 = 20;
const int CHANCE_OF_GEN_8 = 10;
const int CHANCE_OF_GEN_9 = 10;
const int CHANCE_OF_GEN_10 = 10;
//*----------------------------------------------------------------------------------------------------------------------------


// 0 - пусто, 1 - живая, 2 - труп
static int state[SIZE_X][SIZE_Y];
static int energyMap[SIZE_X][SIZE_Y];
static int cellIdMap[SIZE_X][SIZE_Y];

//Для следующий итерации перемещения (чтобы не баговалось при обходе)
static int nextState[SIZE_X][SIZE_Y];
static int nextEnergyMap[SIZE_X][SIZE_Y];
static int nextCellIdMap[SIZE_X][SIZE_Y];

static int corpseMapTimer[SIZE_X][SIZE_Y];

static int arrayOfChance[NUMBER_OF_GENES] = { CHANCE_OF_GEN_1, CHANCE_OF_GEN_2 , CHANCE_OF_GEN_3 ,CHANCE_OF_GEN_4, CHANCE_OF_GEN_5,
                                 CHANCE_OF_GEN_6 ,CHANCE_OF_GEN_7, CHANCE_OF_GEN_8, CHANCE_OF_GEN_9, CHANCE_OF_GEN_10 };

//[tact][cell_id]=id_of_gen (1...10)
static int** matrix_of_gens;
static COLORREF speciesColor[MAX_CELLS]; //speciesColor[cell_id]=цвет клетки собсна

static int currentTact = 0;
static int cycleCount = 0;

static int count_of_cells = 0;

int GetRandomGen() 
{
    int totalWeight = 0; //сумма всех весов
    for (int i = 0; i < NUMBER_OF_GENES; i++) totalWeight += arrayOfChance[i];
    
    // r [0, totalWeight-1]
    int r = rand() % totalWeight;
    //Иду по весам, пока r не станет меньше 0
    for (int i = 0; i < NUMBER_OF_GENES; i++) 
    {
        r -= arrayOfChance[i];
        if (r < 0) return i + 1;
    }
    return NUMBER_OF_GENES; //если пошло через жопу 
}

void initGenes()
{
    matrix_of_gens = new int* [NUMBER_OF_SIGNS];
    for (int t = 0; t < NUMBER_OF_SIGNS; t++)
    {
        matrix_of_gens[t] = new int[MAX_CELLS];
        for (int id = 0; id < MAX_CELLS; id++)
        {
            if (CHANCE_GENERATION) matrix_of_gens[t][id] = GetRandomGen();
            else matrix_of_gens[t][id] = 1+rand()%NUMBER_OF_GENES;
        }
    }
}

void destroyGenes()
{
    for (int t = 0; t < NUMBER_OF_SIGNS; t++) delete[] matrix_of_gens[t];
    delete[] matrix_of_gens;
}

void makeCorpse(int x, int y)
{
    state[x][y] = 2;
    cellIdMap[x][y] = -1;
    energyMap[x][y] = 0;
    corpseMapTimer[x][y] = 1;
}

int layerEnergy(int y)
{
    if (y < SIZE_Y / 3) return ENERGY_FIRST_LAYER;
    if (y < 2 * SIZE_Y / 3) return ENERGY_SECOND_LAYER;
    return ENERGY_THIRD_LAYER;
}

void first_initialization_of_cells()
{
    // очистка поля
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        state[x][y] = 0;
        energyMap[x][y] = 0;
        cellIdMap[x][y] = -1;
        corpseMapTimer[x][y] = 0;
    }

    while (count_of_cells < START_POPULATION)
    {
        int x = rand() % SIZE_X;
        int y = rand() % SIZE_Y;
        if (state[x][y] == 0)
        {
            state[x][y] = 1;
            energyMap[x][y] = START_ENERGY;
            cellIdMap[x][y] = count_of_cells;
            count_of_cells++;
        }
    }

    //Цвета
    for (int id = 0; id < START_POPULATION; id++)
    {
        int a = rand() % 256, b = rand() % 256, c = rand() % 256;
        if ((a != 256 && b != 256 && c != 256) && (a != 165 && b != 42 && c != 42)) speciesColor[id] = RGB(a, b, c);
        else
        {
            a = rand() % 256;
            b = rand() % 256;
            c = rand() % 256;
            speciesColor[id] = RGB(a, b, c);
        }
    }
}

bool similar(int id1, int id2)
{
    int same = 0;
    for (int t = 0; t < NUMBER_OF_SIGNS; t++)
    {
        if (matrix_of_gens[t][id1] == matrix_of_gens[t][id2]) same++;
    }

    int percent = same * 100 / NUMBER_OF_SIGNS;
    return (percent >= SIMILAR_PERCENT_OF_GENS);
}

bool similar_for_sex(int id1, int id2)
{
    int same = 0;
    for (int t = 0; t < NUMBER_OF_SIGNS; t++)
    {
        if (matrix_of_gens[t][id1] == matrix_of_gens[t][id2]) same++;
    }

    int percent = same * 100 / NUMBER_OF_SIGNS;
    return (percent >= SIMILAR_PERCENT_OF_GENS_FOR_SEX);
}

void tact(HWND hWnd)
{
    int t = currentTact;
    static bool protectMap[SIZE_X][SIZE_Y];

    //Энергия за жизнь
    for (int x = 0;x < SIZE_X;x++) for (int y = 0;y < SIZE_Y;y++) if (state[x][y] == 1)
    {
        energyMap[x][y] -= ENERGY_FOR_LIFE;
        if (energyMap[x][y] <= 0) makeCorpse(x, y);
    }

    // Движение (1-вправо 2-влево 3-вверх 4-вниз)
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        nextState[x][y] = state[x][y];
        nextEnergyMap[x][y] = energyMap[x][y];
        nextCellIdMap[x][y] = cellIdMap[x][y];
    }

    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        if (state[x][y] != 1) continue; //не живые скип

        int id = cellIdMap[x][y];
        int gene = matrix_of_gens[t][id];

        if (gene < 1 || gene > 4) continue; //не тот ген тоже скип

        int nx = x, ny = y;
        if (gene == 1) nx++;
        else if (gene == 2) nx--;
        else if (gene == 3) ny--;
        else if (gene == 4) ny++;

        if (nx >= 0 && nx < SIZE_X && ny >= 0 && ny < SIZE_Y && state[nx][ny] == 0 && nextState[nx][ny] == 0)
        {
            nextState[nx][ny] = 1;
            nextEnergyMap[nx][ny] = energyMap[x][y] - ENERGY_FOR_STEP;
            nextCellIdMap[nx][ny] = id;
            nextState[x][y] = 0;
        }

    }

    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        state[x][y] = nextState[x][y];
        energyMap[x][y] = nextEnergyMap[x][y];
        cellIdMap[x][y] = nextCellIdMap[x][y];
    }

    // защита (5)
    for (int x = 0;x < SIZE_X;x++) for (int y = 0;y < SIZE_Y;y++) protectMap[x][y] = false;
    for (int x = 0;x < SIZE_X;x++) for (int y = 0;y < SIZE_Y;y++) if (state[x][y] == 1)
    {
        int id = cellIdMap[x][y];
        if (matrix_of_gens[t][id] == 5) protectMap[x][y] = true;
    }

    // атака (6)
    for (int x = 0;x < SIZE_X;x++) for (int y = 0;y < SIZE_Y;y++) if (state[x][y] == 1)
    {
        int id = cellIdMap[x][y];
        if (matrix_of_gens[t][id] == 6)
        {
            const int offs[4][2] = { {1,0},{-1,0},{0,1},{0,-1} }; //координаты атаки относительно атакующего
            for (auto o : offs)
            {
                int ny = y + o[0], nx = x + o[1];
                if (ny >= 0 && ny < SIZE_Y && nx >= 0 && nx < SIZE_X)
                {
                    if (state[nx][ny] == 1 && !protectMap[nx][ny])
                    {
                        energyMap[x][y] += energyMap[nx][ny];
                        makeCorpse(nx, ny);
                    }
                }
            }
        }
    }

    // Получить энергию от солнца(7)
    for (int y = 0;y < SIZE_Y;++y) for (int x = 0;x < SIZE_X;++x) if (state[x][y] == 1)
    {
        int id = cellIdMap[x][y];
        if (matrix_of_gens[t][id] == 7) energyMap[x][y] += layerEnergy(y);
    }

    // Съесть труп (8)
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        if (state[x][y] != 1) continue;
        int cell_id = cellIdMap[x][y];
        int gen = matrix_of_gens[t][cell_id];

        if (gen != 8) continue;
        const int offs[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
        for (auto o : offs)
        {
            int nx = x + o[1];
            int ny = y + o[0];
            if (nx < 0 || nx >= SIZE_X || ny < 0 || ny >= SIZE_Y) continue;
            if (state[nx][ny] == 2)
            {
                energyMap[x][y] += ENERGY_FROM_BODY;
                state[nx][ny] = 0;
                energyMap[nx][ny] = 0;
                cellIdMap[nx][ny] = -1;
                corpseMapTimer[nx][ny] = 0;
            }
        }
    }

    // Деление(9)
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        if (state[x][y] != 1) continue;
        int cell_id = cellIdMap[x][y];
        int gen = matrix_of_gens[t][cell_id];

        if (gen != 9) continue;
        if (energyMap[x][y] < ENERGY_FOR_SPLIT) continue;

        int parentEnergy = energyMap[x][y];
        int expended = parentEnergy * SPLIT_LESS_ENERGY / 100;
        energyMap[x][y] -= expended;

        const int offs[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
        int ind = rand() % 4;
        int nx = x + offs[ind][0], ny = y + offs[ind][1];
        if (nx < 0 || nx >= SIZE_X || ny < 0 || ny >= SIZE_Y) continue;
        if (state[nx][ny] == 1 || state[nx][ny] == 2) continue;
        state[nx][ny] = 1;
        energyMap[nx][ny] = expended;


        count_of_cells++;
        cellIdMap[nx][ny] = count_of_cells;
        int childId = count_of_cells;
        if (childId >= MAX_CELLS) { --count_of_cells; continue; }

        for (int t = 0; t < NUMBER_OF_SIGNS; t++) matrix_of_gens[t][childId] = matrix_of_gens[t][cellIdMap[x][y]];

        //мутации
        for (int i = 0; i < MUTATIONS_PER_OFFSPRING; i++)
        {
            int rand_number_of_mutation = rand() % NUMBER_OF_SIGNS;
            int rand_index_of_mutation = 1 + rand() % NUMBER_OF_GENES;
            matrix_of_gens[rand_number_of_mutation][childId] = rand_index_of_mutation;
        }

        if (similar(cell_id, childId)) speciesColor[childId] = speciesColor[cell_id];
        else speciesColor[childId] = RGB(rand() % 256, rand() % 256, rand() % 256);
    }

    // Размножение(10)
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        if (state[x][y] != 1) continue;
        int parentId = cellIdMap[x][y];
        int gen = matrix_of_gens[t][parentId];

        if (gen != 10) continue;
        if (energyMap[x][y] < ENERGY_FOR_SEX) continue;

        int x_sec_par = -1, y_sec_par = -1;
        bool found = false;
        for (int dx = -RADIUS_OF_SEX; dx <= RADIUS_OF_SEX && !found; dx++)
        {
            for (int dy = -RADIUS_OF_SEX; dy <= RADIUS_OF_SEX; dy++)
            {                                                  //  0 0
                if (dx == 0 && dy == 0) continue; //себя не надо   ---
                int tx = x + dx, ty = y + dy;
                if (tx < 0 || tx >= SIZE_X || ty < 0 || ty >= SIZE_Y) continue;
                if (state[tx][ty] != 1) continue;
                if (energyMap[tx][ty] < ENERGY_FOR_SEX) continue;
                int secParId = cellIdMap[tx][ty];
                if (!similar_for_sex(parentId, secParId)) continue;

                x_sec_par = tx;
                y_sec_par = ty;
                found = true;
                break;
            }
        }
        if (!found) continue;

        int secParId = cellIdMap[x_sec_par][y_sec_par];

        int contribA = energyMap[x][y] * SEX_LESS_ENERGY / 100;
        int contribB = energyMap[x_sec_par][y_sec_par] * SEX_LESS_ENERGY / 100;
        int energy_for_son = contribA + contribB;
        energyMap[x][y] -= contribA;
        energyMap[x_sec_par][y_sec_par] -= contribB;

        const int offs2[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
        int nx = -1, ny = -1;
        for (int tries = 0; tries < 8; tries++)
        {
            int ind = rand() % 4;
            int tx = x + offs2[ind][0];
            int ty = y + offs2[ind][1];
            if (tx < 0 || tx >= SIZE_X || ty < 0 || ty >= SIZE_Y) continue;
            if (state[tx][ty] == 0)
            {
                nx = tx;
                ny = ty;
                break;
            }
        }
        if (nx < 0 || ny < 0) continue;

        state[nx][ny] = 1;
        energyMap[nx][ny] = energy_for_son;
        count_of_cells++;
        int childId = count_of_cells;
        if (childId >= MAX_CELLS) 
        {
            count_of_cells--;
            state[nx][ny] = 0;
            continue;
        }
        cellIdMap[nx][ny] = childId;

        const int proportions[3] = { 50, 75, 25 };
        int ind_v = rand() % 3;
        int per_f = proportions[ind_v];
        int per_s = 100 - per_f;
        int signs_f = per_f * NUMBER_OF_SIGNS / 100; //сколько генов от parent
        int signs_s = per_s * NUMBER_OF_SIGNS / 100; //сколько генов от secPar

        for (int gi = 0; gi < signs_f; gi++) matrix_of_gens[gi][childId] = matrix_of_gens[gi][parentId];
        for (int gi = signs_f; gi < signs_f + signs_s; gi++) matrix_of_gens[gi][childId] = matrix_of_gens[gi][secParId];

        for (int m = 0; m < MUTATIONS_PER_OFFSPRING; m++)
        {
            int pos = rand() % NUMBER_OF_SIGNS;
            matrix_of_gens[pos][childId] = 1 + rand() % NUMBER_OF_GENES;
        }

        if (BEAUTIFUL_COLORS)
        {
            bool assigned = false;
            for (int i = 0; i < childId; i++) if (similar(i, childId))
            {
                speciesColor[childId] = speciesColor[i];
                assigned = true;
                break;
            }

            if (!assigned) speciesColor[childId] = RGB(rand() % 256, rand() % 256, rand() % 256);
        }

        else
        {
            if (similar(childId, parentId) && similar(childId, secParId)) speciesColor[childId] = speciesColor[parentId];
            else speciesColor[childId] = RGB(rand() % 256, rand() % 256, rand() % 256);
        }

        break;
    }


    //удаляю трупы
    for (int x = 0; x < SIZE_X; x++) for (int y = 0; y < SIZE_Y; y++)
    {
        if (corpseMapTimer[x][y] == 0) continue;
        if (corpseMapTimer[x][y] > TIME_OF_CORPSE) state[x][y] = 0;
        else corpseMapTimer[x][y]++;
    }


    //просто счетчик 
    currentTact++;
    if (currentTact >= NUMBER_OF_SIGNS)
    {
        currentTact = 0;
        cycleCount++;
    }

    wstring title = L"Tact: " + to_wstring(currentTact) + L"  Cycle: " + to_wstring(cycleCount);
    SetWindowTextW(hWnd, title.c_str());
    InvalidateRect(hWnd, nullptr, FALSE);
}

//закраска сетки
void drawGrid(HDC dc)
{
    HBRUSH corpseBrush = CreateSolidBrush(RGB(165, 42, 42)); //коричнеый это труп
    HBRUSH emptyBrush = CreateSolidBrush(RGB(255, 255, 255)); //пустой
    for (int x = 0;x < SIZE_X;x++) for (int y = 0;y < SIZE_Y;y++)
    {
        //координата x левой границы, координата y верхней границы, координата x правой границы, координата y ниджеей границы
        RECT square = { x * SIZE_OF_SQ, y * SIZE_OF_SQ, (x + 1) * SIZE_OF_SQ, (y + 1) * SIZE_OF_SQ }; //генерирую клетку
        if (state[x][y] == 1)
        {
            HBRUSH color = CreateSolidBrush(speciesColor[cellIdMap[x][y]]); //создает кисть цвета клетки
            FillRect(dc, &square, color); //закрась square цветом color
            DeleteObject(color);
        }

        else if (state[x][y] == 2)
        {
            FillRect(dc, &square, corpseBrush);
        }

        else
        {
            FillRect(dc, &square, emptyBrush);
        }
    }

    //Линии разделяющие слои
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(dc, hPen);

    int y1 = SIZE_Y / 3 * SIZE_OF_SQ;
    int y2 = 2 * SIZE_Y / 3 * SIZE_OF_SQ;

    MoveToEx(dc, 0, y1, nullptr); LineTo(dc, SIZE_X * SIZE_OF_SQ, y1); //верхняя
    MoveToEx(dc, 0, y2, nullptr); LineTo(dc, SIZE_X * SIZE_OF_SQ, y2); //нижняя

    SelectObject(dc, oldPen);
    DeleteObject(hPen);

    DeleteObject(corpseBrush);
    DeleteObject(emptyBrush);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        srand((unsigned)time(nullptr));
        initGenes();
        first_initialization_of_cells();
        SetTimer(hWnd, TIMER_ID, TIME_OF_TACT, nullptr);
        return 0;
    }
    case WM_TIMER:
        tact(hWnd);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps; HDC dc = BeginPaint(hWnd, &ps);
        drawGrid(dc); EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ID);
        destroyGenes();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wP, lP);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"Ocean";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);
    RECT rect = { 0,0, SIZE_X * SIZE_OF_SQ, SIZE_Y * SIZE_OF_SQ }; //окно
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, FALSE);
    HWND hWnd = CreateWindowEx(0, wc.lpszClassName, L"Game of Life GA",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInst, nullptr);
    ShowWindow(hWnd, nCmdShow);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}