// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppFunctionalStyleCast

#include "../common/conditions.h"
#if linux_defined

#include "libs.h"

struct ThreadArgs {
    Mat *img;
    int startY;
    int endY;
    double factor;
};

pthread_mutex_t logMutex;

void destroyMutex(pthread_mutex_t *mutex) {
    pthread_mutex_destroy(mutex);
}

void *adjust_contrast_thread(void *lpParameter) {
    auto *args = static_cast<ThreadArgs *>(lpParameter);

    Mat *img = args->img;
    int startY = args->startY;
    int endY = args->endY;
    double factor = args->factor;

    int width = img->cols;
    for (int y = startY; y < endY; ++y) {
        for (int x = 0; x < width; ++x) {
            auto &color = img->at<Vec3b>(y, x);

            // Adjust RGB values based on the contrast factor
            color[2] = clamp(int((color[2] - 128) * factor) + 128, 0, 255); // R
            color[1] = clamp(int((color[1] - 128) * factor) + 128, 0, 255); // G
            color[0] = clamp(int((color[0] - 128) * factor) + 128, 0, 255); // B
        }
    }

    // Add logs after exit
    pthread_mutex_lock(&logMutex);
    cout << "Thread " << pthread_self() << " contrast thread finished successfully" << endl;
    pthread_mutex_unlock(&logMutex);

    pthread_exit(nullptr);
}

void adjust_contrast_multi_threaded(Mat *img, double factor, int numThreads = 1) {
    if (numThreads < 1) {
        throw invalid_argument("numThreads must be greater than 0");
    }

    int height = img->rows;
    int rowsPerThread = height / numThreads;

    vector<pthread_t> threads(numThreads);
    vector<ThreadArgs> args(numThreads);

    for (int i = 0; i < numThreads; ++i) {
        args[i] = ThreadArgs{
            img,
            i * rowsPerThread,
            (i == numThreads - 1) ? height : (i + 1) * rowsPerThread,
            factor
        };
        pthread_create(&threads[i], nullptr, adjust_contrast_thread, &args[i]);
    }

    // Wait for all threads to complete
    for (auto &thread: threads) {
        pthread_join(thread, nullptr);
    }
}

void inner_change_contrast(
    const string &input,
    const string &output,
    double factor,
    int numThreads = 1
) {
    if (factor <= 0 || factor > 2) {
        throw invalid_argument("factor must be in range (0, 2]");
    }

    // Load the image from file
    Mat img = imread(input);
    if (img.empty()) {
        throw invalid_argument("Failed to load input image");
    }

    // Adjust contrast using multiple threads
    adjust_contrast_multi_threaded(&img, factor, numThreads);

    // Save the adjusted image
    imwrite(output, img);
}

void change_contrast(
    const vector<string> &input,
    const vector<string> &output,
    double factor,
    int numThreads
) {
    if (input.size() != output.size()) {
        throw invalid_argument("input and output size mismatch");
    }

    logMutex = PTHREAD_MUTEX_INITIALIZER;

    vector<pthread_t> threads(input.size());
    vector<tuple<string, string, double, int> > args(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        args[i] = make_tuple(input[i], output[i], factor, numThreads);
        pthread_create(&threads[i], nullptr, [](void *param) -> void *{
            auto t_args = *static_cast<tuple<string, string, double, int> *>(param);
            inner_change_contrast(
                get<0>(t_args),
                get<1>(t_args),
                get<2>(t_args),
                get<3>(t_args)
            );
            pthread_exit(nullptr);
        }, &args[i]);
    }

    // Wait for all threads to finish
    for (auto &thread: threads) {
        pthread_join(thread, nullptr);
    }

    destroyMutex(&logMutex);
}

void change_contrast(
    const string &input,
    const string &output,
    double factor,
    int numThreads = 1
) {
    change_contrast(vector{input}, vector{output}, factor, numThreads);
}

#endif
