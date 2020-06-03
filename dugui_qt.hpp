#ifndef DUGUI_QT_WIDGETS
#define DUGUI_QT_WIDGETS
#include "dugui.hpp"

namespace DuGUI {

struct BackendQt : public Backend {
	void setType(WidgetType type) override;
	void setTitle(const std::string& title) override;
	void addValueChangedReacion(const std::function<void(const std::string&)>& reaction) override;
	void addValueChangedReacion(const std::function<void(long long int)>& reaction) override;
	void addValueChangedReacion(const std::function<void(double)>& reaction) override;
	void setValue(const std::string& value) override;
	void setValue(long long int value) override;
	void setValue(double value) override;
	void setBorder(bool border) override;
	void setDefaultText(const std::string& defaultText) override;
	void addElement(const std::shared_ptr<Backend>& element) override;
	void addElement(Backend* element) override;
	void setIfWindow(bool window) override;
	void create() override;
};

} // namespace DuGUI
#endif // DUGUI_QT_WIDGETS
