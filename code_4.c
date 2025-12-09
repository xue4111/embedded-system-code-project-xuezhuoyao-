

/* 
   main.c
   Waveform Generator
   
   --- Code Structure Analysis ---
   1. UI Layer: Handles user menus and input (main_menu, get_user_input).
   2. Logic Layer: configurations (sine, square...) and math (sample_base_waveform).
   3. View Layer: Visualization. 
      * Modified to split Table output (8 points) from Plot resolution (100 points).
   4. Modulation Layer: Applies AM/FM/PWM on top of base waveforms.
   
   Compile: gcc main.c -o main -lm -std=c99 -Wall -Wextra
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* --- Constants --- */
#define DEFAULT_SAMPLES 100     /* Internal resolution for ASCII Plot (High fidelity) */
#define TABLE_SAMPLES 8         /* Display resolution for Table Output (Per requirement) */
#define ASCII_ROWS 21
#define PI 3.14159265358979323846f

/* --- Forward Prototypes --- */
void main_menu(void);
int get_user_input(void);
void select_menu_item(int input);
void print_main_menu(void);
void print_modulation_menu(void);
void print_sine_menu(void);
void print_square_menu(void);
void print_triangle_menu(void);
void print_sawtooth_menu(void);
void go_back_to_main(void);
int is_integer(const char *num);
void menu_item_1(void);
void menu_item_2(void);
void menu_item_3(void);
void menu_item_4(void);

void sine(void);
void square(void);
void triangle(void);
void sawtooth(void);

void sine_plot(void);
void square_plot(void);
void triangle_plot(void);
void sawtooth_plot(void);

void modulation_prompt(int waveform_type);
void modulation_menu_and_run(int waveform_type);
void run_AM(int waveform_type);
void run_FM(int waveform_type);
void run_PWM(int waveform_type);

int parse_phase_input_to_rad(const char *s_in, float *out_rad);
void print_ascii_from_yvals(const float *yvals, int cols, int rows, float amp);
float sample_base_waveform(int waveform_type, float t);

/* 
   --- Variable Analysis (Global State) ---
   These static variables represent the Data Model.
   - fre_*: Frequency in Hz.
   - amp_*: Amplitude in Volts.
   - phase_sin: Phase shift for Sine wave (radians).
   - duty_cycle: 0.0 to 1.0 for Square wave.
*/
static float fre_sin = 1.0f, fre_squ = 1.0f, fre_saw = 1.0f, fre_tra = 1.0f;
static float amp_sin = 1.0f, amp_squ = 1.0f, amp_saw = 1.0f, amp_tra = 1.0f;
static float phase_sin = 0.0f; 
static float duty_cycle = 0.5f, slope = 1.0f;

/* --- Main --- */
int main(int argc, char const *argv[]) {
    (void)argc; (void)argv;
    main_menu();
    return 0;
}

/* ---------- Menu System ---------- */

void main_menu(void) {
    print_main_menu();
    int input = get_user_input();
    select_menu_item(input);
}

int get_user_input(void) {
    int input = 0;
    char input_string[100];
    int valid_input = 0;
    int menu_items = 4;

    do {
        printf("\nSelect a waveform you'd like to generate (1-%d): ", menu_items);
        if (scanf("%99s", input_string) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) { }
            printf("Enter an integer!\n");
            continue;
        }

        valid_input = is_integer(input_string);
        if (!valid_input) {
            printf("Enter an integer!\n");
        } else {
            input = atoi(input_string);
            if (input >= 1 && input <= menu_items) {
                valid_input = 1;
            } else {
                printf("Invalid menu item!\n");
                valid_input = 0;
            }
        }
    } while (!valid_input);

    return input;
}

void select_menu_item(int input) {
    switch (input) {
        case 1: menu_item_1(); break;
        case 2: menu_item_2(); break;
        case 3: menu_item_3(); break;
        case 4: menu_item_4(); break;
        default:
            printf("\nWrong number to select\n");
            go_back_to_main();
            break;
    }
}

void print_main_menu(void) {
    printf("\n----------- waveform generator -----------\n");
    printf("|                                       |\n");
    printf("|   1. sine                              |\n");
    printf("|   2. square                            |\n");
    printf("|   3. triangle                          |\n");
    printf("|   4. sawtooth                          |\n");
    printf("-----------------------------------------\n");
}

void print_sine_menu(void) {
    printf("\n----------- sine settings -----------\n");
    printf("| 1. frequency: %.6f Hz               |\n", fre_sin);
    printf("| 2. amplitude: %.6f V                |\n", amp_sin);
    printf("| 3. phase:     %.6f rad              |\n", phase_sin);
    printf("-------------------------------------\n");
}

void print_square_menu(void) {
    printf("\n----------- square settings ---------\n");
    printf("| 1. frequency: %.6f Hz               |\n", fre_squ);
    printf("| 2. amplitude: %.6f V                |\n", amp_squ);
    printf("| 3. duty cycle: %.6f                  |\n", duty_cycle);
    printf("-------------------------------------\n");
}

void print_triangle_menu(void) {
    printf("\n---------- triangle settings ---------\n");
    printf("| 1. frequency: %.6f Hz               |\n", fre_tra);
    printf("| 2. amplitude: %.6f V                |\n", amp_tra);
    printf("-------------------------------------\n");
}

void print_sawtooth_menu(void) {
    printf("\n---------- sawtooth settings ---------\n");
    printf("| 1. jump amplitude: %.6f V           |\n", amp_saw);
    printf("| 2. slope:          %.6f             |\n", slope);
    printf("-------------------------------------\n");
}

void go_back_to_main(void) {
    char input[100];
    do {
        printf("\nEnter 'b' or 'B' to go back to main menu: ");
        if (scanf("%99s", input) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) { }
            continue;
        }
    } while (input[0] != 'b' && input[0] != 'B');
    main_menu();
}

int is_integer(const char *num) {
    if (num == NULL || num[0] == '\0') return 0;
    int start = 0;
    if (num[0] == '+' || num[0] == '-') {
        start = 1;
        if (num[start] == '\0') return 0;
    }
    for (int i = start; num[i] != '\0'; i++) {
        if (!isdigit((unsigned char)num[i])) return 0;
    }
    return 1;
}

int parse_phase_input_to_rad(const char *s_in, float *out_rad) {
    if (!s_in || !out_rad) return 0;
    char s[128];
    strncpy(s, s_in, sizeof(s)-1);
    s[sizeof(s)-1] = '\0';
    
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start) - 1;
    while (end >= start && isspace((unsigned char)*end)) { *end = '\0'; end--; }

    if ((start[0]=='r' || start[0]=='R') && start[1]==':') {
        double v;
        if (sscanf(start+2, "%lf", &v) == 1) { *out_rad = (float)v; return 1; }
        return 0;
    }
    if ((start[0]=='d' || start[0]=='D') && start[1]==':') {
        double deg;
        if (sscanf(start+2, "%lf", &deg) == 1) { *out_rad = (float)(deg * PI / 180.0f); return 1; }
        return 0;
    }

    {
        char *p = NULL;
        if ((p = strstr(start, "deg")) != NULL) {
            *p = '\0';
            double deg;
            if (sscanf(start, "%lf", &deg) == 1) { *out_rad = (float)(deg * PI / 180.0f); return 1; }
            return 0;
        }
        size_t len = strlen(start);
        if (len > 0 && (start[len-1]=='d' || start[len-1]=='D')) {
            start[len-1] = '\0';
            double deg;
            if (sscanf(start, "%lf", &deg) == 1) { *out_rad = (float)(deg * PI / 180.0f); return 1; }
            return 0;
        }
    }

    {
        char *slash = strchr(start, '/');
        if (slash) {
            *slash = '\0';
            double a=0.0, b=1.0;
            if (sscanf(start, "%lf", &a) == 1 && sscanf(slash+1, "%lf", &b) == 1 && b != 0.0) {
                *out_rad = (float)(a / b);
                return 1;
            }
            return 0;
        }
    }

    {
        double v;
        if (sscanf(start, "%lf", &v) == 1) { *out_rad = (float)v; return 1; }
    }

    return 0;
}

/* ---------- Configuration Handlers ---------- */

void sine(void)
{
    printf("\ninput frequency (Hz): ");
    if (scanf("%f", &fre_sin) != 1) { fre_sin = 1.0f; }
    print_sine_menu();

    printf("\ninput amplitude (V): ");
    if (scanf("%f", &amp_sin) != 1) { amp_sin = 1.0f; }
    print_sine_menu();

    int ch; while ((ch = getchar()) != '\n' && ch != EOF);

    printf("\ninput phase (rad). Examples: 1.57    3.14/2    90deg    d:90    r:1.57\n");
    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), stdin)) { phase_sin = 0.0f; }
    else {
        float rad = 0.0f;
        if (parse_phase_input_to_rad(buf, &rad)) phase_sin = rad;
        else { printf("Failed to parse phase, set to 0.\n"); phase_sin = 0.0f; }
    }
    print_sine_menu();
}

void square(void)
{
    printf("\ninput frequency (Hz): ");
    if (scanf("%f", &fre_squ) != 1) { fre_squ = 1.0f; }
    print_square_menu();

    printf("\ninput amplitude (V): ");
    if (scanf("%f", &amp_squ) != 1) { amp_squ = 1.0f; }
    print_square_menu();

    printf("\ninput duty cycle (0..1): ");
    if (scanf("%f", &duty_cycle) != 1) { duty_cycle = 0.5f; }
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 1.0f) duty_cycle = 1.0f;
    print_square_menu();
}

void triangle(void)
{
    printf("\ninput frequency (Hz): ");
    if (scanf("%f", &fre_tra) != 1) { fre_tra = 1.0f; }
    print_triangle_menu();

    printf("\ninput amplitude (V): ");
    if (scanf("%f", &amp_tra) != 1) { amp_tra = 1.0f; }
    print_triangle_menu();
}

void sawtooth(void)
{
    printf("\ninput jump amplitude (V): ");
    if (scanf("%f", &amp_saw) != 1) { amp_saw = 1.0f; }
    print_sawtooth_menu();

    printf("\ninput slope: ");
    if (scanf("%f", &slope) != 1) { slope = 1.0f; }
    print_sawtooth_menu();
}

/* ---------- ASCII Plotting Helper ---------- */

void print_ascii_from_yvals(const float *yvals, int cols, int rows, float amp)
{
    if (!yvals || cols <= 0 || rows <= 0) return;

    char **canvas = (char **)malloc(rows * sizeof(char *));
    if (!canvas) { perror("malloc canvas"); return; }
    for (int r = 0; r < rows; r++) {
        canvas[r] = (char *)malloc((size_t)(cols + 1));
        if (!canvas[r]) {
            perror("malloc canvas[r]");
            for (int k = 0; k < r; k++) free(canvas[k]);
            free(canvas);
            return;
        }
        for (int c = 0; c < cols; c++) canvas[r][c] = ' ';
        canvas[r][cols] = '\0';
    }

    for (int c = 0; c < cols; c++) {
        float y = yvals[c];
        float frac = 0.5f;
        if (amp != 0.0f) {
            frac = (amp - y) / (2.0f * amp); 
        }
        if (frac < 0.0f) frac = 0.0f;
        if (frac > 1.0f) frac = 1.0f;
        int row = (int)(frac * (rows - 1) + 0.5f);
        if (row < 0) row = 0;
        if (row >= rows) row = rows - 1;
        canvas[row][c] = '*';
    }

    int mid_row = (int)(((amp - 0.0f) / (2.0f * (amp == 0.0f ? 1.0f : amp))) * (rows - 1) + 0.5f);
    if (mid_row < 0) mid_row = 0;
    if (mid_row >= rows) mid_row = rows - 1;
    for (int c = 0; c < cols; c++) if (canvas[mid_row][c] == ' ') canvas[mid_row][c] = '-';

    for (int r = 0; r < rows; r++) {
        printf("%s\n", canvas[r]);
    }

    for (int r = 0; r < rows; r++) free(canvas[r]);
    free(canvas);
}

float sample_base_waveform(int waveform_type, float t)
{
    switch (waveform_type) {
        case 1: {
            return amp_sin * sinf(2.0f * PI * fre_sin * t + phase_sin);
        }
        case 2: {
            if (fre_squ <= 0.0f) return 0.0f;
            float T = 1.0f / fre_squ;
            float pos = fmodf(t, T) / T;
            return (pos < duty_cycle) ? amp_squ : -amp_squ;
        }
        case 3: {
            if (fre_tra <= 0.0f) return 0.0f;
            float T = 1.0f / fre_tra;
            float pos = fmodf(t, T) / T;
            if (pos < 0.5f) return -amp_tra + 4.0f * amp_tra * pos;
            else return 3.0f * amp_tra - 4.0f * amp_tra * pos;
        }
        case 4: {
            if (fre_saw <= 0.0f) return 0.0f;
            float T = 1.0f / fre_saw;
            float frac = fmodf(t, T) / T;
            return -amp_saw + 2.0f * amp_saw * frac;
        }
        default:
            return 0.0f;
    }
}

/* ---------- Modified Plotting Functions ---------- */

/* 
   Function Analysis: 
   Coordinates the calculation and display for Sine wave.
   Crucially, it separates the numeric output (Table) from the visual output (Plot).
   
   Code Structure Analysis:
   1. Validation: Checks frequency > 0.
   2. Table Loop: Runs TABLE_SAMPLES (8) times to print precise numbers.
   3. Plot Loop: Runs DEFAULT_SAMPLES (100) times to populate array for ASCII art.
*/
void sine_plot(void){
    if (fre_sin <= 0.0f) {
        printf("\nFrequency must be > 0!\n");
        return;
    }
    
    // --- 1. Table Output (8 points over 1 period) ---
    printf("\n========== Sine Wave Table (One Period, 8 Samples) ==========\n");
    printf("Frequency = %.6f Hz, Amplitude = %.6f, Phase = %.6f rad\n\n",
           fre_sin, amp_sin, phase_sin);
    printf("t(sec)\t\ty\n");
    
    float period = 1.0f / fre_sin;
    float table_step = period / (float)TABLE_SAMPLES;
    
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        // Calculate y including phase for table accuracy
        float y = amp_sin * sinf(2.0f * PI * fre_sin * t + phase_sin);
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Data Generation (100 points, Silent) ---
    const int N = DEFAULT_SAMPLES; 
    const int rows = ASCII_ROWS;
    float plot_step = period / (float)N;
    
    // Allocate memory for high-res plot data
    float *yval = (float *)malloc(N * sizeof(float));
    if (!yval) { perror("malloc yval"); return; }

    /* Phase shift handling for horizontal alignment in plot */
    float time_shift = 0.0f;
    if (fre_sin != 0.0f) time_shift = phase_sin / (2.0f * PI * fre_sin);

    for (int i = 0; i < N; i++) {
        float t = i * plot_step + time_shift;
        float y = amp_sin * sinf(2.0f * PI * fre_sin * t); 
        yval[i] = y;
        // Note: No printf here. Only filling buffer.
    }

    printf("\n========== Sine Wave ASCII Plot ==========\n");
    print_ascii_from_yvals(yval, N, rows, amp_sin);
    printf("===========================================\n");

    modulation_prompt(1);
    free(yval);
}

void square_plot(void)
{
    if (fre_squ <= 0.0f) { printf("\nFrequency must be > 0!\n"); return; }

    float period = 1.0f / fre_squ;

    // --- 1. Table Output (8 Samples) ---
    printf("\n\n========== Square Wave Table (One Period, 8 Samples) ==========\n");
    printf("Frequency = %.6f Hz, Amplitude = %.6f, Duty = %.6f\n\n",
           fre_squ, amp_squ, duty_cycle);
    printf("t(sec)\t\ty\n");
    
    float table_step = period / (float)TABLE_SAMPLES;
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float y = sample_base_waveform(2, t);
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Generation (100 Samples) ---
    const int N = DEFAULT_SAMPLES;
    float plot_step = period / (float)N;
    float *yval = (float *)malloc(N * sizeof(float));
    if (!yval) { perror("malloc yval"); return; }

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        yval[i] = sample_base_waveform(2, t);
    }

    printf("\n========== Square Wave ASCII Plot ==========\n");
    print_ascii_from_yvals(yval, N, ASCII_ROWS, amp_squ);
    printf("===============================================\n");

    modulation_prompt(2);
    free(yval);
}

void triangle_plot(void)
{
    if (fre_tra <= 0.0f) { printf("\nFrequency must be > 0!\n"); return; }

    float period = 1.0f / fre_tra;

    // --- 1. Table Output ---
    printf("\n========== Triangle Wave Table (One Period, 8 Samples) ==========\n");
    printf("Frequency = %.6f Hz, Amplitude = %.6f\n\n", fre_tra, amp_tra);
    printf("t(sec)\t\ty\n");
    
    float table_step = period / (float)TABLE_SAMPLES;
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float y = sample_base_waveform(3, t);
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Generation ---
    const int N = DEFAULT_SAMPLES;
    float plot_step = period / (float)N;
    float *yval = (float *)malloc(N * sizeof(float));
    if (!yval) { perror("malloc yval"); return; }

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        yval[i] = sample_base_waveform(3, t);
    }

    printf("\n========== Triangle Wave ASCII Plot ==========\n");
    print_ascii_from_yvals(yval, N, ASCII_ROWS, amp_tra);
    printf("===============================================\n");

    modulation_prompt(3);
    free(yval);
}

void sawtooth_plot(void)
{
    if (fre_saw <= 0.0f) { printf("\nFrequency must be > 0!\n"); return; }

    float period = 1.0f / fre_saw;

    // --- 1. Table Output ---
    printf("\n\n========== Sawtooth Wave Table (One Period, 8 Samples) ==========\n");
    printf("Frequency = %.6f Hz, Jump Amp = %.6f, Slope = %.6f\n\n", fre_saw, amp_saw, slope);
    printf("t(sec)\t\ty\n");
    
    float table_step = period / (float)TABLE_SAMPLES;
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float y = sample_base_waveform(4, t);
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Generation ---
    const int N = DEFAULT_SAMPLES;
    float plot_step = period / (float)N;
    float *yval = (float *)malloc(N * sizeof(float));
    if (!yval) { perror("malloc yval"); return; }

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        yval[i] = sample_base_waveform(4, t);
    }

    printf("\n========== Sawtooth Wave ASCII Plot ==========\n");
    print_ascii_from_yvals(yval, N, ASCII_ROWS, amp_saw);
    printf("===============================================\n");

    modulation_prompt(4);
    free(yval);
}

/* ---------- Wrappers ---------- */

void menu_item_1(void) {
    printf("\n>> sine\n");
    sine();
    sine_plot();
    go_back_to_main();
}

void menu_item_2(void) {
    printf("\n>> square\n");
    square();
    square_plot();
    go_back_to_main();
}

void menu_item_3(void) {
    printf("\n>> triangle\n");
    triangle();
    triangle_plot();
    go_back_to_main();
}

void menu_item_4(void) {
    printf("\n>> sawtooth\n");
    sawtooth();
    sawtooth_plot();
    go_back_to_main();
}

/* ---------- Modulation Logic ---------- */

void print_modulation_menu(void)
{
    printf("\n----------- modulation menu -----------\n");
    printf("| 1. AM (Amplitude Modulation)        |\n");
    printf("| 2. FM (Frequency Modulation)        |\n");
    printf("| 3. PWM (Pulse Width Modulation)     |\n");
    printf("---------------------------------------\n");
}

void modulation_prompt(int waveform_type)
{
    char yn = 'n';
    printf("\nDo you want to apply modulation to this waveform? (y/n): ");
    if (scanf(" %c", &yn) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    if (yn == 'y' || yn == 'Y') {
        modulation_menu_and_run(waveform_type);
    }
}

void modulation_menu_and_run(int waveform_type)
{
    print_modulation_menu();
    int choice = 0;
    printf("\nSelect modulation type (1-3): ");
    if (scanf("%d", &choice) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input\n");
        return;
    }
    switch (choice) {
        case 1: run_AM(waveform_type); break;
        case 2: run_FM(waveform_type); break;
        case 3: run_PWM(waveform_type); break;
        default: printf("Invalid modulation choice\n"); break;
    }

    go_back_to_main();
}

/* ---------- Modulation Implementations (Table vs Plot) ---------- */

void run_AM(int waveform_type)
{
    printf("\n=== AM Modulation ===\n");
    float Ac = 1.0f, fc = 1.0f, m = 0.5f;
    printf("Carrier amplitude Ac: "); if (scanf("%f", &Ac) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    printf("Carrier frequency fc (Hz): "); if (scanf("%f", &fc) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    printf("Modulation index m (0..1 recommended): "); if (scanf("%f", &m) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }

    float period = (fc > 0.0f) ? 1.0f / fc : 1.0f;

    // --- 1. Table Output (8 Samples) ---
    printf("\n=== AM Sample Table (One Period, 8 Samples) ===\n");
    printf("t(sec)\t\ty\n");
    float table_step = period / (float)TABLE_SAMPLES;
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f;
        float env = 1.0f + m * xnorm;
        float carrier = sinf(2.0f * PI * fc * t);
        float y = (Ac * env) * carrier;
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Data Generation ---
    const int N = DEFAULT_SAMPLES;
    float *y = (float *)malloc(N * sizeof(float));
    if (!y) { perror("malloc y"); return; }
    float plot_step = period / (float)N;

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f; 
        float env = 1.0f + m * xnorm; 
        float carrier = sinf(2.0f * PI * fc * t);
        y[i] = (Ac * env) * carrier;
    }

    printf("\n=== AM ASCII Plot ===\n");
    print_ascii_from_yvals(y, N, ASCII_ROWS, Ac * (1.0f + fabsf(m)));
    printf("=========================================\n");

    free(y);
}

void run_FM(int waveform_type)
{
    printf("\n=== FM Modulation ===\n");
    float Ac = 1.0f, fc = 1.0f, beta = 1.0f;
    printf("Carrier amplitude Ac: "); if (scanf("%f", &Ac) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    printf("Carrier frequency fc (Hz): "); if (scanf("%f", &fc) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    printf("Modulation index beta (radians, controls deviation): "); if (scanf("%f", &beta) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }

    float period = (fc > 0.0f) ? 1.0f / fc : 1.0f;

    // --- 1. Table Output ---
    printf("\n=== FM Sample Table (One Period, 8 Samples) ===\n");
    printf("t(sec)\t\ty\n");
    float table_step = period / (float)TABLE_SAMPLES;
    
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f;
        float inst_phase = 2.0f * PI * fc * t + beta * xnorm;
        float y = Ac * sinf(inst_phase);
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Data Generation ---
    const int N = DEFAULT_SAMPLES;
    float *y = (float *)malloc(N * sizeof(float));
    if (!y) { perror("malloc y"); return; }
    float plot_step = period / (float)N;

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f;
        float inst_phase = 2.0f * PI * fc * t + beta * xnorm;
        y[i] = Ac * sinf(inst_phase);
    }

    printf("\n=== FM ASCII Plot ===\n");
    print_ascii_from_yvals(y, N, ASCII_ROWS, Ac);
    printf("=========================================\n");

    free(y);
}

void run_PWM(int waveform_type)
{
    printf("\n=== PWM Modulation ===\n");
    float fpwm = 50.0f, Ac = 1.0f;
    printf("PWM carrier frequency fpwm (Hz): "); if (scanf("%f", &fpwm) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }
    printf("Output amplitude Ac (for high level): "); if (scanf("%f", &Ac) != 1) { int c; while ((c=getchar())!=EOF && c!='\n'); }

    float period = (fpwm > 0.0f) ? 1.0f / fpwm : 1.0f;

    // --- 1. Table Output ---
    printf("\n=== PWM Sample Table (One Period, 8 Samples) ===\n");
    printf("t(sec)\t\ty\n");
    float table_step = period / (float)TABLE_SAMPLES;
    
    for (int i = 0; i < TABLE_SAMPLES; i++) {
        float t = i * table_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f;
        float carrier_frac = fmodf(t, period) / period;
        float tri = -1.0f + 2.0f * carrier_frac;
        float y = (xnorm > tri) ? Ac : -Ac;
        printf("%.6f\t%.6f\n", t, y);
    }

    // --- 2. Plot Data Generation ---
    const int N = DEFAULT_SAMPLES;
    float *y = (float *)malloc(N * sizeof(float));
    if (!y) { perror("malloc y"); return; }
    float plot_step = period / (float)N;

    for (int i = 0; i < N; i++) {
        float t = i * plot_step;
        float xt = sample_base_waveform(waveform_type, t);
        float amp_base = 1.0f;
        switch (waveform_type) {
            case 1: amp_base = amp_sin; break;
            case 2: amp_base = amp_squ; break;
            case 3: amp_base = amp_tra; break;
            case 4: amp_base = amp_saw; break;
            default: amp_base = 1.0f; break;
        }
        float xnorm = (amp_base != 0.0f) ? (xt / amp_base) : 0.0f;
        float carrier_frac = fmodf(t, period) / period; 
        float tri = -1.0f + 2.0f * carrier_frac; 

        if (xnorm > tri) y[i] = Ac;
        else y[i] = -Ac;
    }

    printf("\n=== PWM ASCII Plot ===\n");
    print_ascii_from_yvals(y, N, ASCII_ROWS, Ac);
    printf("=========================================\n");

    free(y);
}