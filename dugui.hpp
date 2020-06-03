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
	CheckBox,
	SpinBox
};

struct DuGuiError : std::logic_error {
	using std::logic_error::logic_error;
};

struct Backend {
	virtual void setType(WidgetType widgetType) = 0;
	virtual void setTitle(const std::string& title) = 0;
	virtual void addValueChangedReacion(const std::function<void(const std::string&)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(long long int)>& reaction) = 0;
	virtual void addValueChangedReacion(const std::function<void(double)>& reaction) = 0;
	virtual void setValue(const std::string& value) = 0;
	virtual void setValue(long long int value) = 0;
	virtual void setValue(double value) = 0;
	virtual void setBorder(bool border) = 0;
	virtual void setDefaultText(const std::string& defaultText) = 0;
	virtual void addElement(const std::shared_ptr<Backend>& element) = 0;
	virtual void addElement(Backend* element) = 0;
	virtual void setIfWindow(bool window) = 0;
	virtual void create() = 0;
};

class Widget;

// Properties
struct PropertyTitle{
	std::string title;
};

struct PropertyDefaultText {
	std::string defaultText;
};

struct PropertyNoBorder { };

struct PropertyStandardBorder { };

// Stacking of properties
template <typename... Properties>
struct PropertyGroup { };

template <typename Property, typename... Properties>
struct PropertyGroup<Property, Properties...> {
	PropertyGroup<Properties...> others;
	Property property;
	Widget* parent() const {
		return others.parent();
	}

	PropertyGroup<PropertyTitle, Property, Properties...> title(const std::string& text) {
		return { &this, {text} };
	}
	PropertyGroup<PropertyDefaultText, Property, Properties...> defaultText(const std::string& defaultTextSet) {
		return { &this, {defaultTextSet} };
	}
	PropertyGroup<PropertyNoBorder, Property, Properties...> noBorder() {
		return { &this, {}};
	}
	PropertyGroup<PropertyStandardBorder, Property, Properties...> standardBorder() {
		return { &this, {}};
	}
};

template <>
struct PropertyGroup<> {
	Widget* _parent;
	Widget* parent() const {
		return _parent;
	}
};

// Widgets
class Widget {
	void setTitle(const std::string& title) {
		_backend->setTitle(title);
	}
	friend struct Title;

protected:
	std::shared_ptr<Backend> _backend;

public:
	template <typename... OtherProperties>
	Widget(const PropertyGroup<PropertyTitle, OtherProperties...> properties) : Widget(properties.others) {
		_backend->setTitle(properties.property.title);
	}

	Widget(const PropertyGroup<> properties) {
		_backend = properties._parent->_backend;
	}
};

class Container : public Widget {
	bool _border;
	void setBorder(bool border) {
		_border = border;
	}

protected:
	PropertyGroup<PropertyTitle> title(const std::string& titleSet) {
		return { {this}, {titleSet} };
	}
	PropertyGroup<PropertyDefaultText> defaultText(const std::string& defaultTextSet) {
		return { {this}, {defaultTextSet} };
	}
	PropertyGroup<PropertyNoBorder> noBorder() {
		return { {this}, {}};
	}
	PropertyGroup<PropertyStandardBorder> standardBorder() {
		return { {this}, {}};
	}

public:
	using Widget::Widget;
	template <typename... OtherProperties>
	Container(const PropertyGroup<PropertyNoBorder, OtherProperties...> properties) : Container(properties.others) {
		_border = false;
	}

	friend struct Border;
};

class HBox : public Container {
	template <typename... Properties>
	HBox(const PropertyGroup<Properties...> properties) : Container(properties) {
		_backend->setType(WidgetType::HBox);
	}
};

class VBox : public Container {
	template <typename... Properties>
	VBox(const PropertyGroup<Properties...> properties) : Container(properties) {
		_backend->setType(WidgetType::HBox);
	}
};

template <typename T>
class InputBase : public Widget {
protected:
	T _contents;

public:

	void set(const T& newValue) {
		_backend->setValue(newValue);
	}

	T& operator*() {
		return _contents;
	}
	T* operator->() {
		return &_contents;
	}

	template <typename... OtherProperties>
	InputBase(const PropertyGroup<PropertyDefaultText, OtherProperties...> properties) : Widget(properties.others) {
		_backend->setDefaultText(properties.property.defaultText);
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
	template <typename... Properties>
	InputDerived(const PropertyGroup<Properties...> properties) : Widget(properties) {
		_backend->setType(WidgetType::LineEdit);
		_backend->addValueChangedReacion([this] (const std::string& newText) {
			_contents = newText;
		});
	}

	using InputBase::InputBase;
};

template <typename T>
class InputDerived<T, typename std::enable_if<std::is_integral<T>::value>::type> : public InputBase<T> {
public:
	template <typename... Properties>
	InputDerived(const PropertyGroup<Properties...> properties) : Widget(properties) {
		InputBase<T>::_backend->setType(WidgetType::NumberEdit);
		InputBase<T>::_backend->addValueChangedReacion([this] (long long int newNumber) {
			InputBase<T>::_contents = newNumber;
		});
	}

	using InputBase<T>::InputBase;
};

template <typename T>
using Input = InputDerived<T, void>;

// Pseudo widgets
struct PseudoWidget {
	PseudoWidget(const PropertyGroup<>) { }
};

struct Title : public PseudoWidget {
	template <typename... OtherProperties>
	Title(const PropertyGroup<PropertyTitle, OtherProperties...> properties) : Title(properties.others) {
		properties.parent()->setTitle(properties.property.title);
	}
	using PseudoWidget::PseudoWidget;
};

struct Border : public PseudoWidget {
	template <typename... OtherProperties>
	Border(const PropertyGroup<PropertyNoBorder, OtherProperties...> properties) : Border(properties.others) {
		properties.parent()->setBorder(false);
	}
	template <typename... OtherProperties>
	Border(const PropertyGroup<PropertyStandardBorder, OtherProperties...> properties) : Border(properties.others) {
		properties.parent()->setBorder(true);
	}
	using PseudoWidget::PseudoWidget;
};

} // namespace DuGUI
#endif // DUGUI
