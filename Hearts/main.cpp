#include "Include.h"

bool GetHostOrClient(); // Both

int main()
{
    ImageManager im;

    sf::RenderWindow* window = nullptr; 

    bool hostB = false;

    hostB = GetHostOrClient();

    if (hostB)
    {
        Host host;
        host.Run(window, &im);
    }
    else
    {
        Client client;
        client.Run(window, &im);
    }

    delete window;
    window = nullptr;

    
}

bool GetHostOrClient()
{
    bool ret;
    std::cout << "1 för host och 0 för client: ";
    std::cin >> ret;

    return ret;
}


