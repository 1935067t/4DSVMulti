#include <iostream>
#include <fstream>
#include <kvs/glfw/Application>
#include <kvs/glfw/Screen>
#include <kvs/EventListener>
#include <kvs/Slider>
#include <kvs/Label>
#include <kvs/String>
#include <kvs/Directory>
#include <kvs/FileList>
#include <kvs/ffmpeg/MovieObject>
#include <kvs/ffmpeg/SphericalMovieRenderer>
#include <kvs/CheckBox>
#include <kvs/CheckBoxGroup>

int main(int argc, char **argv)
{
    // Parameters
    kvs::Vec3i dimension{0, 0, 0};
    kvs::Vec3i position{0, 0, 0};
    float frame_rate{0};
    kvs::FileList files{};
    kvs::FileList files2{};
    bool m_loop = false;
    bool m_rotate = false;
    bool isPlay = false;
    bool isPlayPrevious = false;

    // Functions
    auto Read = [&](const std::string filename)
    {
        std::ifstream ifs(filename.c_str());
        if (!ifs.is_open())
        {
            std::cerr << "Cannot open " << filename << std::endl;
            return false;
        }

        std::string dirname{""};
        std::string dirname2{""};
        std::string extension{""};
        ifs >> dirname;
        ifs >> extension;
        ifs >> dimension[0] >> dimension[1] >> dimension[2];
        ifs >> position[0] >> position[1] >> position[2];
        ifs >> frame_rate;
        ifs >> dirname2;
        ifs.close();

        kvs::Directory dir(dirname);
        kvs::Directory dir2(dirname2);
        if (!dir.exists())
        {
            std::cerr << "Not found " << dirname << std::endl;
            return false;
        }

        for (auto file : dir.fileList())
        {
            if (file.extension() == extension)
            {
                files.push_back(file);
            }
        }

        if (!dir2.exists())
        {
            std::cerr << "Not found " << dirname2 << std::endl;
            return false;
        }

        for (auto file : dir2.fileList())
        {
            if (file.extension() == extension)
            {
                files2.push_back(file);
            }
        }

        return true;
    };

    auto Index = [&]()
    {
        const auto &dim = dimension;
        const auto &pos = position;
        return pos.x() + dim.x() * pos.y() + dim.x() * dim.y() * pos.z();
    };

    auto File = [&]()
    {
        const auto file = files[Index()];
        return file.filePath();
    };

    // 追加
    auto File2 = [&]()
    {
        const auto file = files2[Index()];
        return file.filePath();
    };

    // Read config file
    if (!Read(argv[1]))
    {
        std::cerr << "Cannot read " << std::string(argv[1]) << std::endl;
        return 1;
    }

    kvs::glfw::Application app(argc, argv);
    kvs::glfw::Screen screen(&app);
    kvs::glfw::Screen screen2(&app); // 追加

    using Object = kvs::ffmpeg::MovieObject;
    using Renderer = kvs::ffmpeg::SphericalMovieRenderer;
    auto *object = new Object(File());
    auto *object2 = new Object(File2()); // 追加
    auto *renderer = new Renderer();
    auto *renderer2 = new Renderer(); // 追加
    object->setName("Object");
    object2->setName("Object");
    screen.create();
    screen2.create(); // 追加

    kvs::CheckBox loop(&screen);
    loop.setCaption("Loop play");
    loop.anchorToTopRight();
    loop.setFont({kvs::Font::Sans, 18, kvs::RGBColor::White()});
    loop.setMargin(15);
    loop.setState(m_loop);
    loop.stateChanged([&]()
                      { m_loop = loop.state(); });
    // loop.show();

    loop.show();

    kvs::Label label(&screen);
    label.anchorToTopLeft();
    label.setFont({kvs::Font::Sans, 18, kvs::RGBColor::White()});
    label.screenUpdated([&]()
                        {
        auto index = Object::DownCast( screen.scene()->object( "Object" ) )->currentFrameIndex();
        auto frame = kvs::String::From( index, 6, '0' );
        label.setText( "  File: " + files[ Index() ].fileName() );
        label.addText( "  Dimension: " + dimension.format( " x ","","" ) );
        label.addText( "  Position: " + position.format( ", ","( "," )" ) );
        label.addText( "  Frame: " + frame ); });
    label.show();

    kvs::Slider slider(&screen);
    slider.anchorToBottomRight();
    slider.setFont({kvs::Font::Sans, 18, kvs::RGBColor::White()});
    slider.setCaption("Frame: " + kvs::String::From(0, 6, '0'));
    slider.setRange(0, object->numberOfFrames() - 1);
    slider.setValue(0);
    slider.sliderMoved([&]()
                       {
        auto index = kvs::Math::Round( slider.value() );
        auto frame = kvs::String::From( index, 6, '0' );
        slider.setValue( index );
        slider.setCaption( "Frame: " + frame ); });
    slider.sliderPressed([&]()
                         {
        //renderer->pause();
        //スライダーを動かす前動画が再生されていたか
        isPlayPrevious = isPlay;
        isPlay = false; });
    slider.sliderReleased([&]()
                          {
        auto index = kvs::Math::Round( slider.value() );
        auto* o = Object::DownCast( screen.scene()->object( "Object" ) );
        auto *o2 = Object::DownCast(screen2.scene()->object("Object")); // 追加
        o->jumpToFrame( index );
        o2->jumpToFrame(index); // 追加
        screen.redraw();
        screen2.redraw();//追加
        isPlay = isPlayPrevious; });
    slider.show();

    const int interval = 1000 / frame_rate; // msec
    kvs::EventListener event;
    event.keyPressEvent([&](kvs::KeyEvent *e)
                        {
        auto ReplaceObject = [&] ()
        {
            auto index = Object::DownCast( screen.scene()->object( "Object" ) )->currentFrameIndex();
            auto* o = new Object( File() );

            o->setName( "Object" );
            o->jumpToFrame( index );
            screen.scene()->replaceObject( "Object", o );
            object = o;

            auto *o2 = new Object(File2()); // 追加
            o2->setName("Object");
            o2->jumpToFrame(index);
            screen2.scene()->replaceObject("Object", o2);
            object2 = o2;

            screen.redraw();
            screen2.redraw();
        };

        switch ( e->key() )
        {
        case kvs::Key::Left:
        {
            position += kvs::Vec3i{ -1, 0, 0 };
            position.x() = kvs::Math::Max( position.x(), 0 );
            ReplaceObject();
            break;
        }
        case kvs::Key::Right:
        {
            position += kvs::Vec3i{ +1, 0, 0 };
            position.x() = kvs::Math::Min( position.x(), dimension.x() - 1 );
            ReplaceObject();
            break;
        }
        case kvs::Key::Up:
        {
            position += kvs::Vec3i{ 0, +1, 0 };
            position.y() = kvs::Math::Min( position.y(), dimension.y() - 1 );
            ReplaceObject();
            break;
        }
        case kvs::Key::Down:
        {
            position += kvs::Vec3i{ 0, -1, 0 };
            position.y() = kvs::Math::Max( position.y(), 0 );
            ReplaceObject();
            break;
        }

        // addition with S.R  on 4/7 [*1]
        // the case for movement in direction of z-axis.
        case kvs::Key::Comma:
        {
            position += kvs::Vec3i{ 0, 0, -1 };
            position.z() =kvs::Math::Max( position.z(), 0 );
            ReplaceObject();
            break;
        }
        case kvs::Key::Period:
        {
            position += kvs::Vec3i{ 0, 0, +1 };
            position.z() = kvs::Math::Min( position.z(), dimension.z() - 1 );
            ReplaceObject();
            break;
        }
        //  追加機能： ループ再生
        case kvs::Key::l:
        {
            m_loop = !m_loop;
            loop.setState( m_loop );
            screen.redraw();
            break;
        }
        // up to here (*1)

        case kvs::Key::s: 
        { 
          renderer->stop();
          renderer2->stop();
          break; 
        }
        case kvs::Key::Space:
        {
            isPlayPrevious = isPlay;
            isPlay = !isPlay;
            break;
        }
        default: break;
        } });

    event.timerEvent([&](kvs::TimeEvent *e)
                     {
        
        if ( isPlay )
        {
            
            if ( !object->isLastFrame() ) 
            { 
                object->jumpToNextFrame();
                object2->jumpToNextFrame();
                screen.redraw(); 
                screen2.redraw();
                kvs::OpenGL::Flush(); 
            }
            else
            {
                if ( m_loop || !isPlayPrevious)
                {
                    object->jumpToFrame( 0 );
                    object2->jumpToFrame( 0 );
                }
                else
                {
                    isPlay = false;
                }
            }
            
            auto index = Object::DownCast( screen.scene()->object( "Object" ) )->currentFrameIndex();
            auto frame = kvs::String::From( index, 6, '0' );
            slider.setValue( index );
            slider.setCaption( "Frame: " + frame );
            screen.redraw();

            isPlayPrevious = isPlay;
        } },
                     interval);
    //視線方向の同期
    event.mouseMoveEvent([&](kvs::MouseEvent *e)
                         {
        if(m_rotate) 
        {object2->setXform(object->xform()); screen2.redraw();} });
    event.mousePressEvent([&](kvs::MouseEvent *e)
                          { m_rotate = true; });
    event.mouseReleaseEvent([&](kvs::MouseEvent *e)
                            { m_rotate = false; });
    kvs::EventListener event2;
    event2.mouseMoveEvent([&](kvs::MouseEvent *e)
                         {
        if(m_rotate) 
        {object->setXform(object2->xform()); screen.redraw();} });
    event2.mousePressEvent([&](kvs::MouseEvent *e)
                          { m_rotate = true; });
    event2.mouseReleaseEvent([&](kvs::MouseEvent *e)
                            { m_rotate = false; });
    screen.addEvent(&event);
    screen2.addEvent(&event2);

    screen.registerObject(object, renderer);
    screen2.registerObject(object2, renderer2);
    // screen2.setControlTargetToLight();
    screen.show();
    screen2.show();

    return app.run();
}