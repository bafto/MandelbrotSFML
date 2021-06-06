#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include <complex>
#include <thread>
#include <sstream>
#include <iostream>

const unsigned int width = 1500, height = 1000;
unsigned int maxIterations = 124;//127; //can be changed, but this value is nearly perfect so don't touch it or the image will become ugly
long double zoom = 1.0;//0.004;
sf::Vector2<long double> offset(-0.7, 0.0);

sf::Image image;
sf::Texture texture;
sf::Sprite sprite;
sf::Text text;
sf::Font font;

std::vector<sf::Color> colors;

unsigned int mandelbrot(std::complex<long double>, const unsigned int);
sf::Color getColor(int);
void generateColors(const unsigned int);
void updateImageSlice(int miny, int maxy);


template<typename T>
std::string to_string(const T val, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << val;
    return out.str();
}

template<typename T>
T map(T val, T min1, T max1, T min2, T max2)
{
    return min2 + ((val - min1) * (max2 - min2)) / (max1 - min1);
}

int main()
{
    std::cout << std::numeric_limits<long double>::digits10;
    sf::RenderWindow wnd(sf::VideoMode(width, height), "Mandelbrot");
    image.create(width, height);
    font.loadFromFile("arial.ttf");
    text.setFont(font);
    text.setCharacterSize(18);
    text.setPosition(10, 10);
    generateColors(maxIterations);
    while (wnd.isOpen())
    {
        bool shouldCalculate = true;
        sf::Event e;
        while (wnd.waitEvent(e))
        {
            shouldCalculate = true;
            if (e.type == sf::Event::Closed)
            {
                wnd.close();
                shouldCalculate = false;
            }
            if (e.type == sf::Event::MouseButtonPressed)
            {
                break;
            }
            if (e.type == sf::Event::KeyPressed)
            {
                if (e.key.code == sf::Keyboard::Enter)
                    image.saveToFile("mandelbrot.png");

                if (e.key.code == sf::Keyboard::W)
                    offset.y -= 140 * zoom;
                if (e.key.code == sf::Keyboard::S)
                    offset.y += 140 * zoom;
                if (e.key.code == sf::Keyboard::A)
                    offset.x -= 140 * zoom;
                if (e.key.code == sf::Keyboard::D)
                    offset.x += 140 * zoom;

                if (e.key.code == sf::Keyboard::Up)
                    zoom *= 0.5;
                if (e.key.code == sf::Keyboard::Down)
                    zoom /= 0.5;
                if (e.key.code == sf::Keyboard::Left)
                {
                    if (maxIterations > 50)
                    {
                        maxIterations -= 50;
                        generateColors(maxIterations);
                        text.setString("Calculated in 0 seconds\nMax Iterations: " + std::to_string(maxIterations) + "\nZoom: " + to_string(zoom, 15));
                    }
                    shouldCalculate = false;
                }
                if (e.key.code == sf::Keyboard::Right)
                {
                    if (maxIterations < 100000)
                    {
                        maxIterations += 50;
                        generateColors(maxIterations);
                        text.setString("Calculated in 0 seconds\nMax Iterations: " + std::to_string(maxIterations) + "\nZoom: " + to_string(zoom, 15));
                    }
                    shouldCalculate = false;
                }
                break;
            }
        }

        if (shouldCalculate)
        {
            sf::Clock timer;
            const unsigned int step = height / 16;//std::thread::hardware_concurrency();
            std::vector<std::thread> threads;
            for (int y = 0; y < height; y += step)
            {
                threads.push_back(std::thread(updateImageSlice, y, std::min(y + step, height)));
            }
            for (auto& thread : threads)
                thread.join();

            text.setString("Calculated in " + std::to_string(timer.getElapsedTime().asSeconds()) + " seconds\nMax Iterations: " + std::to_string(maxIterations) + "\nZoom: " + to_string(zoom, 15));
            texture.loadFromImage(image);
            sprite.setTexture(texture);
        }

        wnd.clear();

        wnd.draw(sprite);
        wnd.draw(text);

        wnd.display();
    }


    return 0;
}

void updateImageSlice(int miny, int maxy)
{
    for (int x = 0; x < width; x++)
    {
        for (int y = miny; y < maxy; y++)
        {
            std::complex<long double> i((x - width / 2.0) * zoom + offset.x, (y - height / 2.0) * zoom + offset.y);
            unsigned int value = mandelbrot(i, maxIterations);
            //vertices[x + y * width] = sf::Vertex(sf::Vector2f((float)x, (float)y), getColor(value));
            image.setPixel(x, y, getColor(value));
        }
    }
}

void generateColors(const unsigned int maxIterations)
{
    colors.resize(maxIterations + 2);
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        float t = float(i) / maxIterations;
        float r = 9.0 * (1.0 - t) * t * t * t;
        float g = 15.0 * (1.0 - t) * (1.0 - t) * t * t;
        float b = 8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t;
        r = map(r, -1.0f, 1.0f, 0.0f, 255.f);
        g = map(g, -1.0f, 1.0f, 0.0f, 255.f);
        b = map(b, -1.0f, 1.0f, 0.0f, 255.f);
        colors[i] = sf::Color(r, g, b);
    }
    colors[maxIterations + 1] = sf::Color::Black;
}

unsigned int mandelbrot(std::complex<long double> i, const unsigned int maxIterations)
{
    unsigned int count = 0;
    std::complex<long double> j = i;
    while (j.real() * j.real() + j.imag() * j.imag() < 4.0 && count < maxIterations)
    {
        j = j * j + i;
        count++;
    }

    return count == maxIterations ? count + 1 : count;
}

sf::Color getColor(int value)
{
    return colors[value];
}