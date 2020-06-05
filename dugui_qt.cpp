#include "dugui_qt.hpp"
#include <QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace DuGUI;

class DuGUI::BackendQtWindow : public QMainWindow {

};

void BackendQt::create(const std::shared_ptr<StartupProperties>& properties) {
	_type = properties->widgetType;

	auto makeContainer = [&] () -> QWidget* {
		if (properties->windowed) {
			_container.window = new BackendQtWindow;
			return _container.window;
		} else {
			_container.nonWindow = new QWidget;
			return _container.nonWindow;
		}
	};

	auto wrapIfNeeded = [&] (QWidget* wrapped) {
		if (_auxiliary.description) return; // Done by parent
		QWidget* container = makeContainer();
		container->setLayout(new QHBoxLayout());
		_auxiliary.description = new QLabel(QString::fromStdString(properties->title));
		container->layout()->addWidget(_auxiliary.description);
		container->layout()->addWidget(wrapped);
	};

	switch (_type) {
	case WidgetType::Unset:
	case WidgetType::Formulaire: {
		QWidget* making = makeContainer();
		QGridLayout* layout = new QGridLayout(making);
		making->setLayout(layout);

		_auxiliary.children = new std::vector<std::shared_ptr<BackendQt>>();
		int line = 0;
		properties->foreachChildren([&] (Widget* it) {
			QLabel* label = new QLabel(QString::fromStdString(it->_properties.properties->title), making);
			layout->addWidget(label, line, 0);
			auto childBackend = std::dynamic_pointer_cast<BackendQt>(it->_backend);
			childBackend->_auxiliary.description = label;
			it->_backend->create(it->_properties.properties);
			_auxiliary.children->push_back(childBackend);
			if (childBackend->_container.dummy)
				layout->addWidget(childBackend->_container.nonWindow, line, 1);
			else if (childBackend->_widget.dummy)
				layout->addWidget(childBackend->_widget.dummy, line, 1);
			line++;
		});
		break;
	} case WidgetType::HBox:
	case WidgetType::VBox: {
		QWidget* making = makeContainer();
		QLayout* layout = nullptr;
		if (_type == WidgetType::VBox)
			layout = new QVBoxLayout(making);
		else
			layout = new QHBoxLayout(making);
		making->setLayout(layout);

		_auxiliary.children = new std::vector<std::shared_ptr<BackendQt>>();
		properties->foreachChildren([&] (Widget* it) {
			auto childBackend = std::dynamic_pointer_cast<BackendQt>(it->_backend);
			childBackend->create(it->_properties.properties);
			_auxiliary.children->push_back(childBackend);
			if (childBackend->_container.dummy)
				layout->addWidget(childBackend->_container.nonWindow);
			else if (childBackend->_widget.dummy)
				layout->addWidget(childBackend->_widget.dummy);
		});
		break;
	} case WidgetType::LineEdit:
		_widget.lineEdit = new QLineEdit();
		wrapIfNeeded(_widget.lineEdit);
		setValue(properties->stringValue);
		break;
	case WidgetType::NumberEdit:
		_widget.lineEdit = new QLineEdit();
		_widget.lineEdit->setValidator(new QIntValidator(_widget.lineEdit));
		wrapIfNeeded(_widget.lineEdit);
		setValue(properties->intValue);
		break;
	case WidgetType::FloatEdit:
		_widget.lineEdit = new QLineEdit();
		_widget.lineEdit->setValidator(new QDoubleValidator(_widget.lineEdit));
		wrapIfNeeded(_widget.lineEdit);
		setValue(properties->doubleValue);
		break;
	case WidgetType::CheckBox:
		_widget.checkBox = new QCheckBox();
		wrapIfNeeded(_widget.checkBox);
		break;
	case WidgetType::SpinBox:
		_widget.spinBox = new QSpinBox();
		wrapIfNeeded(_widget.spinBox);
		break;
	default:
		throw DuGuiError("Widget type not supported by Qt backend");
	}

	if (!properties->title.empty())
		setTitle(properties->title);
	if (properties->stringReaction)
		addValueChangedReacion(properties->stringReaction);
	if (properties->intReaction)
		addValueChangedReacion(properties->intReaction);
	if (properties->doubleReaction)
		addValueChangedReacion(properties->doubleReaction);

	if (properties->windowed)
		_container.window->show();
};

void BackendQt::setTitle(const std::string& title) {
	switch (_type) {
	case WidgetType::LineEdit:
	case WidgetType::NumberEdit:
	case WidgetType::FloatEdit:
	case WidgetType::CheckBox:
	case WidgetType::SpinBox:
		_auxiliary.description->setText(QString::fromStdString(title));
		break;
	default:
		throw DuGuiError("Widget type not supported by Qt backend");
	}
};

void BackendQt::addValueChangedReacion(const std::function<void(const std::string&)>& reaction) {
	switch (_type) {
	case WidgetType::LineEdit:
		QObject::connect(_widget.lineEdit, &QLineEdit::editingFinished, [reaction, edit = _widget.lineEdit] () {
			reaction(edit->text().toStdString());
		});
		break;
	default:
		throw DuGuiError("Can't provide a string callback to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::addValueChangedReacion(const std::function<void(long long int)>& reaction) {
	switch (_type) {
	case WidgetType::NumberEdit:
		QObject::connect(_widget.lineEdit, &QLineEdit::editingFinished, [reaction, edit = _widget.lineEdit] () {
			reaction(edit->text().toLongLong());
		});
		break;
	case WidgetType::CheckBox:
		QObject::connect(_widget.checkBox, &QCheckBox::clicked, [reaction] (bool selected) {
			reaction(selected);
		});
		break;
	case WidgetType::SpinBox:
		QObject::connect(_widget.spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [reaction] (int value) {
			reaction(value);
		});
		break;
	default:
		throw DuGuiError("Can't provide an int callback to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::addValueChangedReacion(const std::function<void(double)>& reaction) {
	switch (_type) {
	case WidgetType::FloatEdit:
		QObject::connect(_widget.lineEdit, &QLineEdit::editingFinished, [reaction, edit = _widget.lineEdit] () {
			reaction(edit->text().toLongLong());
		});
		break;
	default:
		throw DuGuiError("Can't provide a float callback to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::addReaction(const std::function<void()>& reaction) {
	auto reactToClick = [&] (auto clicked, auto signal) {
		QObject::connect(clicked, signal, reaction);
	};
	switch (_type) {
	case WidgetType::CheckBox:
		reactToClick(_widget.checkBox, &QCheckBox::clicked);
		break;
	case WidgetType::Button:
		reactToClick(_widget.button, &QPushButton::clicked);
		break;
	default:
		throw DuGuiError("Can't provide an click callback to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::setValue(const std::string& value) {
	switch (_type) {
	case WidgetType::LineEdit:
		_widget.lineEdit->setText(QString::fromStdString(value));
		break;
	default:
		throw DuGuiError("Can't set a string value to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::setValue(long long int value) {
	switch (_type) {
	case WidgetType::NumberEdit:
		_widget.lineEdit->setText(QString::fromStdString(std::to_string(value)));
		break;
	default:
		throw DuGuiError("Can't set an integer value to a widget of type " + std::to_string(int(_type)));
	}
};

void BackendQt::setValue(double value) {
	switch (_type) {
	case WidgetType::FloatEdit:
		_widget.lineEdit->setText(QString::fromStdString(std::to_string(value)));
		break;
	default:
		throw DuGuiError("Can't set a floating point value to a widget of type " + std::to_string(int(_type)));
		throw DuGuiError("Can't set a floating point value to a widget of type " + std::to_string(int(_type)));
	}
};

std::shared_ptr<Backend> BackendQt::createAnotherElement() {
	return std::make_shared<BackendQt>();
};
