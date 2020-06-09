#ifndef DUGUI
#define DUGUI

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <iostream>

namespace DuGUI {

enum class WidgetType {
	Unset,
	Formulaire,
	HBox,
	VBox,
	LineEdit,
	NumberEdit,
	FloatEdit,
	CheckBox,
	SpinBox,
	Button
};

struct DuGuiError : std::logic_error {
	using std::logic_error::logic_error;
};

struct Widget;
struct PropertyGroup;

struct Backend {
	struct StartupProperties {
		WidgetType widgetType = WidgetType::Unset;
		std::string title;
		std::function<void(const std::string&)> stringReaction;
		std::function<void(long long int)> intReaction;
		std::function<void(double)> doubleReaction;
		std::function<void()> reaction;
		std::string stringValue;
		long long int intValue = 0;
		double doubleValue = 0;
		bool border = false;
		bool windowed = false;
		std::string placeholderText;
		std::vector<std::shared_ptr<Widget>> childrenShared;
		std::vector<Widget*> childrenStatic;
		template <typename T>
		void foreachChildren(T operation) {
			for (auto& it : childrenShared)
				operation(it.get());
			for (auto& it : childrenStatic)
				operation(it);
		}
	};

	virtual void create(StartupProperties* properties) = 0;
	virtual void setTitle(const std::string& title) = 0;
	virtual void addValueChangedReacion(const std::function<void(const std::string&)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(long long int)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(double)>& reaction) = 0;
	virtual void addReaction(const std::function<void()>& reaction) = 0;
	virtual void setValue(const std::string& value) = 0;
	virtual void setValue(long long int value) = 0;
	virtual void setValue(double value) = 0;
	virtual void close() = 0;
	virtual std::shared_ptr<Backend> createAnotherElement() = 0;
};

// Setting properties
struct VBox;
struct HBox;
struct PropertyGroup {
	template <typename Value>
	void reactionToChangeGeneric(std::function<void(Value)>& location, const std::function<void(Value)>& assigned) {
		if (location)
			location = [original = location, assigned] (Value value) {
				original(value);
				assigned(value);
			};
		else
			location = assigned;
	}

	std::shared_ptr<Backend::StartupProperties> properties = std::make_shared<Backend::StartupProperties>();
	Widget* parent;

	PropertyGroup title(const std::string& text) {
		properties->title = text;
		return *this;
	}
	PropertyGroup placeholderText(const std::string& placeholderTextSet) {
		properties->placeholderText = placeholderTextSet;
		return *this;
	}
	PropertyGroup defaultValue(const std::string& value) {
		properties->stringValue = value;
		return *this;
	}
	PropertyGroup defaultValue(long long int value) {
		properties->intValue = value;
		return *this;
	}
	PropertyGroup defaultValue(double value) {
		properties->doubleValue = value;
		return *this;
	}
	PropertyGroup noBorder() {
		properties->border = false;
		return *this;
	}
	PropertyGroup standardBorder() {
		properties->border = true;
		return *this;
	}
	PropertyGroup reaction(const std::function<void()>& assigned) {
		if (properties->reaction)
			properties->reaction = [original = properties->reaction, assigned] {
				original();
				assigned();
			};
		else
			properties->reaction = assigned;
		return *this;
	}
	PropertyGroup reactionToChange(const std::function<void(const std::string&)>& assigned) {
		reactionToChangeGeneric(properties->stringReaction, assigned);
		return *this;
	}
	PropertyGroup reactionToChange(const std::function<void(long long int)>& assigned) {
		reactionToChangeGeneric(properties->intReaction, assigned);
		return *this;
	}
	PropertyGroup reactionToChange(const std::function<void(double)>& assigned) {
		reactionToChangeGeneric(properties->doubleReaction, assigned);
		return *this;
	}
};

// Widgets
struct Widget {
	PropertyGroup _properties;

	void setTitle(const std::string& title) {
		properties()->title = title;
	}

	std::shared_ptr<Backend> _backend;
	Backend::StartupProperties* properties() {
		return _properties.properties.get();
	}

	// pseudo widget
	struct Title {
		Title(const PropertyGroup& properties) {
			properties.parent->setTitle(properties.properties->title);
		}
	};

	void run(Backend& parentBackend) {
		properties()->windowed = true;
		propagateBackends(parentBackend);
		_backend->create(properties());
		_properties.properties.reset();
	}

	void propagateBackends(Backend& parentBackend) {
		if (!_backend)
			_backend = parentBackend.createAnotherElement();
		_properties.properties->foreachChildren([this] (Widget* it) {
			it->propagateBackends(*_backend);
		});
	}

	void close() {
		if (!_backend)
			throw DuGuiError("Closing a window before it's open");
		_backend->close();
	}

	Widget(const PropertyGroup& props) : _properties(props) {
		_properties.parent->properties()->childrenStatic.push_back(this);
	}
	Widget() : _properties({ std::make_shared<Backend::StartupProperties>(), this }) {
	}
};

struct Container : public Widget {
	bool _border = false;
	void setBorder(bool border) {
		_border = border;
	}

	PropertyGroup makeChildProperties() {
		return { std::make_shared<Backend::StartupProperties>(), this };
	}

protected:
	PropertyGroup title(const std::string& titleSet) {
		return makeChildProperties().title(titleSet);
	}
	PropertyGroup placeholderText(const std::string& placeholderTextSet) {
		return makeChildProperties().placeholderText(placeholderTextSet);
	}
	PropertyGroup noBorder() {
		return makeChildProperties().noBorder();
	}
	PropertyGroup standardBorder() {
		return makeChildProperties().standardBorder();
	}
	template <typename T>
	PropertyGroup reaction(const std::function<void(T)>& assigned) {
		return makeChildProperties().reaction(assigned);
	}

public:
	Container(const PropertyGroup& props, WidgetType type) : Widget(props) {
		properties()->widgetType = type;
	}
	Container(WidgetType type) {
		properties()->widgetType = type;
	}

	// pseudo widget
	struct Border {
		Border(const PropertyGroup& properties) {
			static_cast<Container*>(properties.parent)->setBorder(properties.properties->border);
		}
	};
};

struct HBox : public Container {
	HBox(const PropertyGroup& props) : Container(props, WidgetType::HBox) {
	}
	HBox() : Container(WidgetType::HBox) {
	}
};

struct VBox : public Container {
	VBox(const PropertyGroup& props) : Container(props, WidgetType::VBox) {
	}
	VBox() : Container(WidgetType::VBox) {
	}
};

struct Formulaire : public Container {
	Formulaire(const PropertyGroup& props) : Container(props, WidgetType::Formulaire) {
	}
	Formulaire() : Container(WidgetType::Formulaire) {
	}
};

template <typename T>
class InputBase : public Widget {
protected:
	T _contents;

public:
	InputBase(const PropertyGroup& props) : Widget(props) {
	}

	void set(const T& newValue) {
		_backend->setValue(newValue);
	}

	const T& operator*() const {
		return _contents;
	}
	const T* operator->() const {
		return *_contents;
	}
	T& operator=(const T& assigned) {
		_contents = assigned;
		if (_backend)
			_backend->setValue(assigned);
		return _contents;
	}

	using Widget::Widget;
};

template <typename T, typename SFINAE>
class InputDerived {
	static_assert(std::is_arithmetic<T>::value, "Binding input to an incorrect type");
};

template <>
class InputDerived<std::string, void> : public InputBase<std::string> {
public:
	InputDerived(const PropertyGroup& properties) : InputBase(properties) {
		Widget::properties()->widgetType = WidgetType::LineEdit;
		properties.properties->stringReaction = [this] (const std::string& newText) {
			_contents = newText;
		};
	}
	std::string& operator=(const std::string& assigned) {
		if (properties())
			properties()->stringValue = assigned;
		return InputBase<std::string>::operator=(assigned);
	}

	InputDerived() {
		Widget::properties()->widgetType = WidgetType::LineEdit;
		properties()->stringReaction = [this] (const std::string& newText) {
			_contents = newText;
		};
	}
};

template <typename T>
class InputDerived<T, typename std::enable_if<std::is_integral<T>::value>::type> : public InputBase<T> {
public:
	InputDerived(const PropertyGroup& props) : InputBase<T>(props) {
		Widget::properties()->widgetType = WidgetType::NumberEdit;
		InputBase<T>::properties()->intReaction = [this] (long long int newNumber) {
			InputBase<T>::_contents = newNumber;
		};
		InputBase<T>::_contents = 0;
	}
	T& operator=(T assigned) {
		if (InputBase<T>::properties())
			InputBase<T>::properties()->stringValue = assigned;
		return InputBase<std::string>::operator=(assigned);
	}
	InputDerived() {
		Widget::properties()->widgetType = WidgetType::NumberEdit;
		InputBase<T>::properties()->intReaction = [this] (long long int newNumber) {
			InputBase<T>::_contents = newNumber;
		};
	}
};

template <typename T>
using Input = InputDerived<T, void>;

class Button : public Widget {
public:
	Button(const PropertyGroup& propertiesSet) : Widget(propertiesSet) {
		properties()->widgetType = WidgetType::Button;
	}

	void operator=(const std::function<void()> reaction) {
		if (_backend)
			_backend->addReaction(reaction);
		else
			_properties.reaction(reaction);
	}
	Button() {
		properties()->widgetType = WidgetType::Button;
	}
};

} // namespace DuGUI
#endif // DUGUI
