#ifndef STARTUPVIEW_H
#define STARTUPVIEW_H

#include <QWidget>

class QAbstractItemModel;

namespace Ui {
	class StartUpView;
}


namespace UserInterface
{
	/**
	 * @brief Класс формы приветствия программы
	 */
	class StartUpView : public QWidget
	{
		Q_OBJECT

	public:
		explicit StartUpView(QWidget *parent = 0);
		~StartUpView();

		/**
		 * @brief Панель инструментов представления
		 */
		QWidget* toolbar() const;

		/**
		 * @brief Установить список доступных проектов
		 */
		void setRemoteProjects(QAbstractItemModel* _remoteProjectsModel);

	signals:
		/**
		 * @brief Нажата кнопка создать проект
		 */
		void createProjectClicked();

		/**
		 * @brief Выбран один из проектов из облака для открытия
		 */
		void openRemoteProjectClicked(const QModelIndex& _projectIndex);

	protected:
		/**
		 * @brief Переопределяется для фиксации события когда мышка покидает виджет недавних проектов
		 */
		bool eventFilter(QObject* _watched, QEvent* _event);

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения для формы
		 */
		void initConnections();

		/**
		 * @brief Настроить внешний вид
		 */
		void initStyleSheet();

	private:
		/**
		 * @brief Интерфейс представления
		 */
		Ui::StartUpView* m_ui;
	};
}

#endif // STARTUPVIEW_H
