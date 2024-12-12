// WindowManager.h
class WindowManager {
public:
    static WindowManager& getInstance() {
        static WindowManager instance;
        return instance;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getAspectRatio() const { return static_cast<float>(width) / height; }

    void setSize(int newWidth, int newHeight) {
        width = newWidth;
        height = newHeight;
    }

private:
    WindowManager() : width(800), height(600) {}

    int width;
    int height;

    // 복사 방지
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
};