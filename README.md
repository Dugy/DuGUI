# DuGUI
A façade over Qt Widgets (and possibly other libraries) allowing to create GUIs in C++ with minimal amount code. It requires C++17.

## Usage
It's indented to need minimal code, requiring writing only the absolutely necessary information:
```C++
using namespace DuGUI
struct NameWindow : Formulaire {
	Title t = title("Set address");
	struct : HBox {
		Input<std::string> first = placeholderText("First name");
		Input<std::string> last = placeholderText("Last name");
	} name = {{ noBorder().title("Name") }};
	Input<std::string> address = title("Address");
	struct : HBox {
		Input<std::string> city = placeholderText("City").defaultValue("Brno");
		Input<int> code = placeholderText("Postal code");
	} city = {{ noBorder().title("City") }};
	Button submit = title("Submit");
};
```

It communicates through an interface class that can use any GUI library. I have implemented only a wrapper above Qt so far.
```C++
	QApplication a(argc, argv);

	DuGUI::BackendQt backend;
	NameWindow window;
	window.run(backend);
```
The `run()` method blocks the thread until the window is closed.

Callbacks can be set in the GUI class too, but also from outside if it's public.
```C++
	window.submit = [&window] {
		std::string together = *window.name.first + " " + *window.name.last +
			*window.address + ", " + std::to_string(*window.city.code) + " " + *window.city.city;
		std::cout << together << std::endl;
		window.close();
	};
  ```

The code above produces this window:

![A screenshot](screenshot.png)
