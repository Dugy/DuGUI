#ifndef DUGUI
#define DUGUI

#include <string>
#include <vector>
#include <functional>
#include <memory>

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

struct Backend {
	struct StartupProperties {
		WidgetType widgetType;
		std::string title;
		std::function<void(const std::string&)> stringReaction;
		std::function<void(long long int)> intReaction;
		std::function<void(double)> doubleReaction;
		std::function<void()> reaction;
		std::string stringValue;
		long long int intValue;
		double doubleValue;
		bool border;
		bool windowed;
		std::string defaultText;
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

	virtual void create(const std::shared_ptr<StartupProperties>& properties) = 0;
	virtual void setTitle(const std::string& title) = 0;
	virtual void addValueChangedReacion(const std::function<void(const std::string&)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(long long int)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(double)>& reaction) = 0;
	virtual void addReaction(const std::function<void()>& reaction) = 0;
	virtual void setValue(const std::string& value) = 0;
	virtual void setValue(long long int value) = 0;
	virtual void setValue(double value) = 0;
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

	std::shared_ptr<Backend::StartupProperties> properties;
	Widget* parent;

	PropertyGroup title(const std::string& text) {
		properties->title = text;
		return *this;
	}
	PropertyGroup defaultText(const std::string& defaultTextSet) {
		properties->defaultText = defaultTextSet;
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

	operator HBox();
	operator VBox();
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

	static void _setup(Widget* self) {
		self->_properties.parent->properties()->childrenStatic.push_back(self);
	}

	// pseudo widget
	struct Title {
		Title(const PropertyGroup& properties) {
			properties.parent->setTitle(properties.properties->title);
		}
	};

	void run(Backend& parentBackend) {
		_backend = parentBackend.createAnotherElement();
		_backend->create(_properties.properties);
		_properties.properties.reset();
	}
};

class Container : public Widget {
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
	PropertyGroup defaultText(const std::string& defaultTextSet) {
		return makeChildProperties().defaultText(defaultTextSet);
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
	using Widget::Widget;

	// pseudo widget
	struct Border {
		Border(const PropertyGroup& properties) {
			static_cast<Container*>(properties.parent)->setBorder(properties.properties->border);
		}
	};
};

struct HBox : public Container {
	static void setup(HBox* self) {
		self->_properties.properties->widgetType = WidgetType::HBox;
	}
};

inline PropertyGroup::operator HBox() {
	HBox made;
	made._properties = *this;
	Widget::_setup(&made);
	HBox::setup(&made);
	return made;
}

struct VBox : public Container {
	static void setup(VBox* self) {
		self->_properties.properties->widgetType = WidgetType::VBox;
	}
};

inline PropertyGroup::operator VBox() {
	VBox made;
	made._properties = *this;
	Widget::_setup(&made);
	VBox::setup(&made);
	return made;
}

template <typename T>
class InputBase : public Widget {
protected:
	T _contents;
	InputBase(const PropertyGroup& properties) {
		_properties = properties;
		Widget::_setup(this);
	}

public:

	void set(const T& newValue) {
		_backend->setValue(newValue);
	}

	operator T() {
		return _contents;
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
		properties.properties->widgetType = WidgetType::LineEdit;
		_backend->addValueChangedReacion([this] (const std::string& newText) {
			_contents = newText;
		});
	}
	std::string& operator=(const std::string& assigned) {
		if (properties())
			properties()->stringValue = assigned;
		return InputBase<std::string>::operator=(assigned);
	}

	using InputBase::InputBase;
};

template <typename T>
class InputDerived<T, typename std::enable_if<std::is_integral<T>::value>::type> : public InputBase<T> {
public:
	InputDerived(const PropertyGroup& properties) : InputBase<T>(properties) {
		InputBase<T>::properties()->widgetType = WidgetType::NumberEdit;
		InputBase<T>::properties()->intReaction = [this] (long long int newNumber) {
			InputBase<T>::_contents = newNumber;
		};
	}
	T& operator=(T assigned) {
		if (InputBase<T>::properties())
			InputBase<T>::properties()->stringValue = assigned;
		return InputBase<std::string>::operator=(assigned);
	}

	using InputBase<T>::InputBase;
};

class Button : public Widget {
public:
	Button(const PropertyGroup& propertiesSet) {
		_properties = propertiesSet;
		properties()->widgetType = WidgetType::Button;
		Widget::_setup(this);
	}

	void operator=(const std::function<void()> reaction) {
		_backend->addReaction(reaction);
	}
};

template <typename T>
using Input = InputDerived<T, void>;

} // namespace DuGUI
#endif // DUGUI
