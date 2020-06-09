#ifndef DUGUI_QT_WIDGETS
#define DUGUI_QT_WIDGETS
#include "dugui.hpp"

class QPushButton;
class QCheckBox;
class QLineEdit;
class QSlider;
class QSpinBox;
class QWidget;
class QBoxLayout;
class QGridLayout;
class QLabel;

namespace DuGUI {

class BackendQtWindow;

struct BackendQt final : public Backend {
	WidgetType _type;
	bool _windowed;
	union {
		QWidget* dummy = nullptr;
		QPushButton* button;
		QCheckBox* checkBox;
		QLineEdit* lineEdit;
		QSlider* slider;
		QSpinBox* spinBox;
	} _widget;

	union {
		QWidget* dummy = nullptr;
		BackendQtWindow* window;
		QWidget* nonWindow;
	} _container;

	union {
		void* dummy = nullptr;
		QLabel* description;
		std::vector<std::shared_ptr<BackendQt>>* children;
	} _auxiliary;
private:

	void create(StartupProperties* properties) override;
	void setTitle(const std::string& title) override;
	void addValueChangedReacion(const std::function<void(const std::string&)>& reaction) override;
	void addValueChangedReacion(const std::function<void(long long int)>& reaction) override;
	void addValueChangedReacion(const std::function<void(double)>& reaction) override;
	void addReaction(const std::function<void()>& reaction) override;
	void setValue(const std::string& value) override;
	void setValue(long long int value) override;
	void setValue(double value) override;
	void close() override;
	std::shared_ptr<Backend> createAnotherElement() override;
};

} // namespace DuGUI
#endif // DUGUI_QT_WIDGETS
