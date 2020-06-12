#include "dugui.hpp"
#include "dugui_qt.hpp"
#include <QApplication>

using namespace DuGUI;

struct NameWindow : Formulaire {
	Title t = title("Set address");
	struct: HBox {
		Input<std::string> first = placeholderText("First name");
		Input<std::string> last = placeholderText("Last name");
	} name = {{ noBorder().title("Name") }};
	Input<std::string> address = title("Address");
	struct: HBox {
		Input<std::string> city = placeholderText("City").defaultValue("Brno");
		Input<int> code = placeholderText("Postal code");
	} city = {{ noBorder().title("City") }};
	Button submit = title("Submit");
};

struct Test : Formulaire {
	Title t = title("Stuff");
	Input<int> value = title("Value");
	struct: HBox {
		Button less = title("Less").reaction([this] {
			static_cast<Test*>(parent())->value = *static_cast<Test*>(parent())->value - 1;
		});
		Button more = title("More").reaction(&Test::increase);
	} manipulation = {{ noBorder().title("Manipulation") }};
	void increase() {
		value = *value + 1;
	}
	Button submit = title("Done").reaction([this] {
		close();
	});
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	DuGUI::BackendQt backend;
	Test window;
//	window.submit = [&window] {
//		std::string together = *window.name.first + " " + *window.name.last + " " +
//			*window.address + ", " + std::to_string(*window.city.code) + " " + *window.city.city;
//		std::cout << together << std::endl;
//		window.close();
//	};
	window.run(backend);


	return 0;
}
