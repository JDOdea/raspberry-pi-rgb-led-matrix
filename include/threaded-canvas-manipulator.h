// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

// Utility base class for continuously updating the canvas.

// Note: consider removing, as real applications likely have something
// similar, but this might not be quite usable.
// Since it is just a few lines of code, it is probably better
// implemented in the application for readability.
//
// So for simplicity of the API, consider ThreadedCanvasManipulator deprecated.
// However, it shall remain for the sake of learning and practice.

#ifndef RPI_THREADED_CANVAS_MANIPULATOR_H
#define RPI_THREADED_CANVAS_MANIPULATOR_H

#include "thread.h"
#include "canvas.h"

namespace rgb_matrix
{
    //
    // Typically, your programs will create a canvas and then update the image
    // in a loop. If you want to do stuff in parallel, then this utility class
    // helps you do that. Also a demo for how to use the Thread class.
    //
    // Extend it, then just implement Run(). Example:
    /*
        class MyDemo : public ThreadedCanvasManipulator {
        public:
            class MyDemo(Canvas *canvas) : ThreadedCanvasManipulator(canvas) {}
            virtual void Run() {
                unsigned char c;
                while (running()) {
                    // Calculate next frame.
                    c++;
                    for (int x = 0; x < canvas()->width(); ++x) {
                        for (int y = 0; y < canvas()->height(); ++y) {
                            canvas()->SetPixel(x, y, c, c, c);
                        }
                    }
                    usleep(15 * 1000);
                }
            }
        };

        // Later, in your main method.
        RGBMatrix *matrix = RGBMatrix::CreateFromOptions(...);
        MyDemo *demo = new MyDemo(matrix);
        demo->Start();  // Start doing things.
        // This now runs in the background, you can do other things here,
        // e.g. acquiring new data or simply wait. But for waiting, you wouldn't
        // need a thread in the first place.
        demo->Stop();
        delete demo;
    */
    class ThreadedCanvasManipulator : public Thread
    {
    public:
        ThreadedCanvasManipulator(Canvas *m) : running_(false), canvas_(m) {}
        virtual ~ThreadedCanvasManipulator() { Stop(); }

        virtual void Start(int realtime_priority = 0, uint32_t affinity_mask = 0)
        {
            {
                MutexLock l(&mutex_);
                running_ = true;
            }
            Thread::Start(realtime_priority, affinity_mask);
        }

        // Stop the thread at the next possible time Run() checks the running_ flag.
        void Stop()
        {
            MutexLock l(&mutex_);
            running_ = false;
        }

        // Implement this and run while running() returns true.
        virtual void Run() = 0;

    protected:
        inline Canvas *canvas() { return canvas_; }
        inline bool running()
        {
            MutexLock l(&mutex_);
            return running_;
        }

    private:
        Mutex mutex_;
        bool running_;
        Canvas *const canvas_;
    };
} // namespace rgb_matrix

#endif // RPI_THREADED_CANVAS_MANIPULATOR_H