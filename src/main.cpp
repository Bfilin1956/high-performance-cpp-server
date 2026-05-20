#include <cstdio>
#include <exception>
#include <attaServer/App.h>

int main() {
    try {
        App app;
        app.run();
    }catch (std::exception &e) {
        printf("Body exception: %s", e.what());
    }
}
