#ifndef SCENARIOSCENESYNOPSISMANAGER_H
#define SCENARIOSCENESYNOPSISMANAGER_H

#include <QObject>

namespace UserInterface {
	class ScenarioSceneDescription;
}


namespace ManagementLayer
{
	/**
	 * @brief Управляющий редактором описания сцены
	 */
	class ScenarioSceneDescriptionManager : public QObject
	{
		Q_OBJECT

	public:
		explicit ScenarioSceneDescriptionManager(QObject* _parent, QWidget* _parentWidget);

		QWidget* view() const;

		/**
		 * @brief Установить заголовок редактора
		 */
		void setHeader(const QString& _header);

		/**
		 * @brief Установить описание
		 */
		void setDescription(const QString& _description);

		/**
		 * @brief Установить режим работы со сценарием
		 */
		void setCommentOnly(bool _isCommentOnly);

	signals:
		/**
		 * @brief Текст описания изменился
		 */
		void descriptionChanged(const QString& _description);

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения
		 */
		void initConnections();

	private:
		/**
		 * @brief Редактор описания
		 */
		UserInterface::ScenarioSceneDescription* m_view;
	};
}

#endif // SCENARIOSCENESYNOPSISMANAGER_H
