#include <cstdlib>
#include <cassert>
#include <iostream>
#include <array>
#include <string>
#include <tuple>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

using namespace std;
using namespace sf;

const std::string path_image(PATH_IMG);

union my_color
{
	unsigned f;
	std::uint8_t color[4];
};

const my_color color_values[] = {
	{0xFF0000}, {0x00FF00}, {0x0000FF}, {0xFFFF00}, {0xFF00FF}, {0x00FFFF}, {0x000000}, {0x800000}, {0x008000}, {0x000080}, {0x808000}, {0x800080}, {0x008080}, {0x808080}, {0xC00000}, {0x00C000}, {0x0000C0}, {0xC0C000}, {0xC000C0}, {0x00C0C0}, {0xC0C0C0}, {0x400000}, {0x004000}, {0x000040}, {0x404000}, {0x400040}, {0x004040}, {0x404040}, {0x200000}, {0x002000}, {0x000020}, {0x202000}, {0x200020}, {0x002020}, {0x202020}, {0x600000}, {0x006000}, {0x000060}, {0x606000}, {0x600060}, {0x006060}, {0x606060}, {0xA00000}, {0x00A000}, {0x0000A0}, {0xA0A000}, {0xA000A0}, {0x00A0A0}, {0xA0A0A0}, {0xE00000}, {0x00E000}, {0x0000E0}, {0xE0E000}, {0xE000E0}, {0x00E0E0}, {0xE0E0E0}};

	const sf::Font font(path_image+"arial.ttf");

class My_text : public sf::Text
{
public:
	My_text() : sf::Text(font) {}

private:
};

constexpr size_t nb_bars = 5;

int main()
{
	std::array<sf::RectangleShape, nb_bars> rectangles;
	std::array<My_text, nb_bars> textes;

	float position_y = 10;
	int i = 0;

	assert(sizeof(color_values) / sizeof(color_values[0]) > rectangles.size());

	for (auto [irec, itext] = std::tuple{rectangles.begin(), textes.begin()}; irec != rectangles.end(); ++irec, ++itext)
	{
		const my_color &c = color_values[++i];
		irec->setSize(sf::Vector2f(1, 10));
		irec->setPosition(sf::Vector2f(10, position_y));
		irec->setFillColor(sf::Color(c.color[2], c.color[1], c.color[0]));

		itext->setFont(font);
		itext->setFillColor(sf::Color(c.color[2], c.color[1], c.color[0]));
		itext->setCharacterSize(12);
		position_y += 20;
	}

	size_t which_bar = 0;

	rectangles[which_bar].setOutlineThickness(1);
	const sf::Color outlinecolor(255, 255, 255);
	rectangles[which_bar].setOutlineColor(outlinecolor);

	const Vector2u WINDOW_SIZE(800, static_cast<unsigned>(position_y));

	const float window_width = static_cast<float>(WINDOW_SIZE.x - 100);

	const float position_text = window_width + 5;

	RenderWindow app(VideoMode({WINDOW_SIZE.x, WINDOW_SIZE.y}, 32), "My Windows");

	constexpr int incr = 2;

	while (app.isOpen())
	{
		// Event event;
		while(const std::optional event = app.pollEvent())
		{
			if ((event->is<sf::Event::KeyPressed>()&&event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)||
			event->is<sf::Event::Closed>())
			{
				app.close();
			}

			if (event->is<sf::Event::KeyPressed>())
			{
				auto code = event->getIf<sf::Event::KeyPressed>()->code;
				// switch (event.key.code)
				switch (code)
				{
				case sf::Keyboard::Key::Escape:
					app.close();
					break;
				case sf::Keyboard::Key::Left:
				{
					auto size_rectangle = rectangles[which_bar].getSize();
					if (1 < size_rectangle.x)
					{
						rectangles[which_bar].setSize(sf::Vector2f(size_rectangle.x - incr, size_rectangle.y));
					}
					break;
				}

				case sf::Keyboard::Key::Right:
				{
					auto size_rectangle = rectangles[which_bar].getSize();
					if (size_rectangle.x < window_width)
					{
						rectangles[which_bar].setSize(sf::Vector2f(size_rectangle.x + incr, size_rectangle.y));
					}
					break;
				}

				case sf::Keyboard::Key::Up:
					rectangles[which_bar].setOutlineThickness(0);
					if (which_bar == 0)
					{
						which_bar = rectangles.size() - 1;
					}
					else
					{
						--which_bar;
					}
					rectangles[which_bar].setOutlineThickness(1);
					rectangles[which_bar].setOutlineColor(outlinecolor);
					break;

				case sf::Keyboard::Key::Down:
					rectangles[which_bar].setOutlineThickness(0);
					if (which_bar == rectangles.size() - 1)
					{
						which_bar = 0;
					}
					else
					{
						++which_bar;
					}
					rectangles[which_bar].setOutlineThickness(1);
					rectangles[which_bar].setOutlineColor(outlinecolor);
					break;
				default:
					break;
				}
			}
		}

		app.clear();

		for (auto [irec, itext] = std::tuple{rectangles.begin(), textes.begin()}; irec != rectangles.end(); ++irec, ++itext)
		{
			app.draw(*irec);
			itext->setPosition(sf::Vector2f(position_text, irec->getPosition().y));
			itext->setString(std::to_string(irec->getSize().x));
			app.draw(*itext);
		}
		app.display();
	}
	return EXIT_SUCCESS;
}
