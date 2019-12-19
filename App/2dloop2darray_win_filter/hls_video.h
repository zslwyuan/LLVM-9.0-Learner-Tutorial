namespace hls
{

/* Template class of Window */
template <int ROWS, int COLS, typename T> class Window
{
  public:
    Window(){};

    /* Window main APIs */
    void shift_pixels_left();
    void shift_pixels_right();
    void shift_pixels_up();
    void shift_pixels_down();
    void insert_pixel(T value, int row, int col);
    void insert_row(T value[COLS], int row);
    void insert_top_row(T value[COLS]);
    void insert_bottom_row(T value[COLS]);
    void insert_col(T value[ROWS], int col);
    void insert_left_col(T value[ROWS]);
    void insert_right_col(T value[ROWS]);
    T getval(int row, int col);
    T operator()(int row, int col);

    /* Back compatible APIs */
    void shift_left();
    void shift_right();
    void shift_up();
    void shift_down();
    void insert(T value, int row, int col);
    void insert_top(T value[COLS]);
    void insert_bottom(T value[COLS]);
    void insert_left(T value[ROWS]);
    void insert_right(T value[ROWS]);
    // T& getval(int row, int col);
    // T& operator ()(int row, int col);

    T val[ROWS][COLS];
};

/* Template class of Line Buffer */
template <int ROWS, int COLS, typename T, int RESHAPE = 0> class LineBuffer;

template <int ROWS, int COLS, typename T> class LineBuffer<ROWS, COLS, T, 0>
{
  public:
    LineBuffer(){};
    /* LineBuffer main APIs */
    void shift_pixels_up(int col);
    void shift_pixels_down(int col);
    void insert_bottom_row(T value, int col);
    void insert_top_row(T value, int col);
    void get_col(T value[ROWS], int col);
    T &getval(int row, int col);
    T &operator()(int row, int col);

    /* Back compatible APIs */
    void shift_up(int col);
    void shift_down(int col);
    void insert_bottom(T value, int col);
    void insert_top(T value, int col);
    // T& getval(int row, int col);
    // T& operator ()(int row, int col);

    T val[ROWS][COLS];
};

template <int ROWS, int COLS, typename T> class LineBuffer<ROWS, COLS, T, 1>
{
  public:
    LineBuffer(){};
    /* LineBuffer main APIs */
    void shift_pixels_up(int col);
    void shift_pixels_down(int col);
    void insert_bottom_row(T value, int col);
    void insert_top_row(T value, int col);
    void get_col(T value[ROWS], int col);
    T getval(int row, int col);
    T operator()(int row, int col);

    /* Back compatible APIs */
    void shift_up(int col);
    void shift_down(int col);
    void insert_bottom(T value, int col);
    void insert_top(T value, int col);
    // T& getval(int row, int col);
    // T& operator ()(int row, int col);

    T val[ROWS][COLS];
};

} // namespace hls
