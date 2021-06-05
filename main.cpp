#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include <complex>
#include <iostream>
#include <thread>

const unsigned int width = 1500, height = 1000;
unsigned int maxIterations = 120;//127; //can be changed, but this value is nearly perfect so don't touch it or the image will become ugly
long double zoom = 0.004;
sf::Vector2<long double> offset(-0.7, 0.0);

sf::VertexArray vertices;
sf::Image image;
sf::Texture texture;
sf::Sprite sprite;

std::vector<sf::Color> colors;

unsigned int mandelbrot(std::complex<long double>, const unsigned int);
sf::Color getColor(int);
void generateColors(const unsigned int);
void updateImageSlice(int miny, int maxy);


template<typename T>
T map(T val, T min1, T max1, T min2, T max2)
{
    return min2 + ((val - min1) * (max2 - min2)) / (max1 - min1);
}

int main()
{
    std::cout << "Supported number of threads: " << std::thread::hardware_concurrency() << "\n";
    sf::RenderWindow wnd(sf::VideoMode(width, height), "Mandelbrot");
    vertices.setPrimitiveType(sf::PrimitiveType::Points);
    vertices.resize(width * height);
    image.create(width, height);
    generateColors(maxIterations);
    while (wnd.isOpen())
    {
        bool shouldCalculate = true;
        sf::Event e;
        while (wnd.waitEvent(e))
        {
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
                break;
            }
        }

        if (shouldCalculate)
        {
            std::cout << "Beginning to calculate\n";
            sf::Clock timer;
            const unsigned int step = height / 16;//std::thread::hardware_concurrency();
            std::vector<std::thread> threads;
            for (int y = 0; y < height; y += step)
            {
                threads.push_back(std::thread(updateImageSlice, y, std::min(y + step, height)));
            }
            for (auto& thread : threads)
                thread.join();
            std::cout << "Finished calculating in " + std::to_string(timer.getElapsedTime().asSeconds()) + " seconds\n";

            texture.loadFromImage(image);
            sprite.setTexture(texture);

            wnd.clear();

            wnd.draw(sprite);

            wnd.display();
        }
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
    for (unsigned int i = 0; i <= maxIterations; i++)
    {
        double quotient = (double)i / (double)maxIterations;
        double color = quotient;
        if (quotient > 1.0)
            color = 1.0;
        else if (quotient < 0.0)
            color = 0.0;
        color = map(color, 0.0, 1.0, 0.0, 255.0);
        if (quotient > 0.5)
            colors[i] = sf::Color(color, 255, color);
        else
            colors[i] = sf::Color(0, color, 0);
    }
    colors[maxIterations + 1] = sf::Color::Black;
}

unsigned int mandelbrot(std::complex<long double> i, const unsigned int maxIterations)
{
    unsigned int count = 0;
    std::complex j = i;
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