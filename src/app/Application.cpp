#include "app/Application.h"
#include "app/GameState.h"
#include "app/PauseState.h"
#include "utils/Assert.h"
#include "utils/File.h"
#include "utils/Log.h"
#include "3rdparty/json11/json11.hpp"
#include <string>
#include <chrono>
#include <thread>

namespace pg {

namespace {
    std::chrono::duration<float, std::ratio<1,1>> TargetDeltaTime{ 0.016667f };
}

void Application::run() { 
    initialize_();

    running_ = true;
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::ratio<1,1>> time( 0.0f );

    while( running_ ) {
        auto newTime = std::chrono::steady_clock::now();
        std::chrono::duration<float, std::ratio<1,1>> dt = newTime - currentTime;
        currentTime = std::chrono::steady_clock::now();
        /*
         * Update mouse current mouse coordinates here
         * */
        updateContext_();
        context_.time_ = time.count();
        /*
         * Handle events here
         * */
        SDL_Event event;
        while( SDL_PollEvent( &event ) ) {
            stateStack_.handleEvent( event );
        }
        /*
         * A state might have called quits
         * */
        if ( !context_.running ) {
            running_ = false;
        }

        stateStack_.update( dt.count() );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        stateStack_.render( dt.count() );

        window_.display();
        time += dt;

        /*
         * Sleep for the remainder of the frame, if we have time for it
         * */
        auto remainder = TargetDeltaTime - (std::chrono::steady_clock::now() - newTime);
        if ( remainder.count() > 0.0f ) {
            std::this_thread::sleep_for( remainder );
        }
    }
}

void Application::initialize_() {
    auto json = pg::FileToString( "config.json" );
    std::string error{""};
    auto obj = json11::Json::parse( json, error ).object_items();
    auto window = obj["window"].object_items();
    auto opengl = window["opengl"].object_items();

    TargetDeltaTime = std::chrono::duration<float, std::ratio<1,1>>( 1.0f /  obj["frameRate"].number_value() );

    WindowSettings settings{};
    settings.width = window["width"].int_value();
    settings.height = window["height"].int_value();
    settings.name = window["name"].string_value();
    settings.glMajor = opengl["major"].int_value();
    settings.glMinor = opengl["minor"].int_value();
    settings.stencilBits = opengl["stencil_bits"].int_value();
    settings.depthBits = opengl["depth_bits"].int_value();
    settings.multisampleBuffer = opengl["multisample_buffers"].int_value();
    settings.multisampleSamples = opengl["multisample_samples"].int_value();

    window_.initialize( settings );

    context_.window = &window_;

    stateStack_.registerState< GameState >( states::Game );
    stateStack_.registerState< PauseState >( states::Pause );
    stateStack_.pushState( states::Game );
}

void Application::updateContext_() {
    // update real-time input
    int oldx = context_.mouse_.x;
    int oldy = context_.mouse_.y;
    int newx, newy;
    SDL_GetMouseState( &newx, &newy );
    context_.mouse_.dx = newx - oldx;
    context_.mouse_.dy = newy - oldy;
    context_.mouse_.x = newx;
    context_.mouse_.y = newy;
}

}