#include <iostream>
#include <map>
#include "object.h"
#include "board.h"
#include "klibriry.h"
#include <SFML/Graphics.hpp>
void CoordinatesAdapter (int* a){
    const int zx=30;
    const int zy=52;
    a[0]=(a[0]+a[0]-a[1])*zx+217;
    a[1]=198-a[1]*zy;
}
int main()
{
    const int radius = 5;
    board B(radius);
    char d;
    int* a;
    std::map <char, int> direct = {{'a', 0}, {'w', 1}, {'e', 2}, {'d', 3}, {'x', 4}, {'z', 5}, {'s', 6}};
    std::map <char, int> :: iterator it;
    hero H(B.center, Step, 1);
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(452, 411), "Magic Fight");
    // Load a sprite to display
    sf::Texture texture;
    if (!texture.loadFromFile("img/board.png"))
        return EXIT_FAILURE;
    sf::Sprite sprite(texture);
     sf::Texture hero_texture;
    if (!hero_texture.loadFromFile("img/hero.png"))
        return EXIT_FAILURE;
    sf::Sprite hero(hero_texture);

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                d = event.key.code + 97;
                //ks::clear();
                //B.print();
                //std::cout<<std::endl;
                //std::cin>>d;
                it = direct.find(d);
                if (it != direct.end()){
                    H.skill->DoIt(it->second);
                }
            }
        }
        // Clear screen
        window.clear();
        a = H.position->GetCoordinate();
        std::cout << a[0] << " " << a[1]<< std::endl;
        CoordinatesAdapter(a);
        hero.setPosition(a[0], a[1]);
        delete[] a;
        // Draw the sprite
        window.draw(sprite);
        window.draw(hero);
        // Update the window
        window.display();
    }
    return EXIT_SUCCESS;
}
