#include "dugui.hpp"
#include "dugui_qt.hpp"

using namespace DuGUI;

struct NameWindow : DuGUI::VBox {
	Title t = title("Set address");
//	struct : HBox {
		Input<std::string> first = defaultText("First name");
		Input<std::string> last = defaultText("Last name");
//	} name = { noBorder() };
	Input<std::string> address = defaultText("Address");
//	struct : HBox {
		Input<std::string> city = defaultText("City");
		Input<int> code = defaultText("Postal code");
//	} city = { noBorder() };
//	Button submit = title("Submit");
};

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
//	MainWindow w;
//	w.show();

	DuGUI::BackendQt();


	return a.exec();
}
