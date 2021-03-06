#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>


namespace UserInterface {
	class ApplicationView;
}


namespace ManagementLayer
{
	class ProjectsManager;
	class LoginManager;
	class MenuManager;
	class CabinManager;
	class StartUpManager;
	class ScenarioManager;
	class SynchronizationManager;
	class SettingsManager;


	/**
	 * @brief Управляющий приложением
	 */
	class ApplicationManager : public QObject
	{
		Q_OBJECT

	public:
		explicit ApplicationManager(QObject *parent = 0);
		~ApplicationManager();

	public slots:
		/**
		 * @brief Запуск приложения
		 */
		void exec();

	private:
		/**
		 * @brief Обновить списки проектов на стартовой странице
		 */
		void aboutUpdateProjectsList();

		/**
		 * @brief Создать новый
		 */
		void createNewProject(const QString& _projectName);

		/**
		 * @brief Сохранить в файл
		 */
		void saveProject();

//		/**
//		 * @brief Загрузить
//		 *
//		 * Если имя файла не задано, то будет вызван диалог выбора файла
//		 */
//		void aboutLoad(const QString& _fileName = QString());

//		/**
//		 * @brief Открыть справку
//		 */
//		void aboutShowHelp();

		/**
		 * @brief Загрузить из списка
		 */
		/** @{ */
		void aboutLoadFromRecent(const QModelIndex& _projectIndex);
		void aboutLoadFromRemote(const QModelIndex& _projectIndex);
		/** @} */

//		/**
//		 * @brief Отобразить индикатор активной синхронизации
//		 */
//		void aboutShowSyncActiveIndicator();

		/**
		 * @brief Синхронизация остановленна с ошибкой
		 */
		void syncClosedWithError(int errorCode, const QString& _error = QString::null);

//		/**
//		 * @brief Импортировать документ
//		 */
//		void aboutImport();

//		/**
//		 * @brief Экспортировать документ
//		 */
//		void aboutExportTo();

//		/**
//		 * @brief Предварительный просмотр и печать
//		 */
//		void aboutPrintPreview();

		/**
		 * @brief Закрыть приложение
		 */
		void aboutExit();

		/**
		 * @brief Проект был изменён
		 */
		void aboutProjectChanged();

//		/**
//		 * @brief Включить/выключить полноэкранный режим
//		 */
//		void aboutShowFullscreen();

//		/**
//		 * @brief Подготовить документ для подсчёта статистики
//		 */
//		void aboutPrepareScenarioForStatistics();

//	private:
//		/**
//		 * @brief Загрузить состояние окна
//		 */
//		void loadViewState();

//		/**
//		 * @brief Сохранить состояние окна
//		 */
//		void saveViewState();

		/**
		 * @brief Если проект был изменён, но не сохранён предложить пользователю сохранить его
		 *
		 * Возвращает true, если пользователь хочет (Да) или не хочет (Нет) сохранять
		 * и false, если пользователь передумал (Отмена)
		 */
		bool saveIfNeeded();

		/**
		 * @brief Настроить текущий проект для редактирования
		 */
		void goToEditCurrentProject();

		/**
		 * @brief Закрыть текущий проект
		 */
		void closeCurrentProject();

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения
		 */
		void initConnections();

		/**
		 * @brief Настроить внешний вид
		 */
		void initStyleSheet();

		/**
		 * @brief Перезагрузить настройки приложения
		 */
		void reloadApplicationSettings();

		/**
		 * @brief Обновить заголовок окна
		 */
		void updateWindowTitle();

	private:
		/**
		 * @brief Главное окно приложения
		 */
		UserInterface::ApplicationView* m_view;

		/**
		 * @brief Управляющий проектами
		 */
		ProjectsManager* m_projectsManager;

		/**
		 * @brief Управляющий страницей авторизации
		 */
		LoginManager* m_loginManager;

		/**
		 * @brief Управляющий меню навигации приложения
		 */
		MenuManager* m_menuManager;

		/**
		 * @brief Управляющий личным кабинетом пользователя
		 */
		CabinManager* m_cabinManager;

		/**
		 * @brief Управляющий страницей с проектами
		 */
		StartUpManager* m_startUpManager;

		/**
		 * @brief Управляющий сценарием
		 */
		ScenarioManager* m_scenarioManager;

//		/**
//		 * @brief Управляющий персонажами
//		 */
//		CharactersManager* m_charactersManager;

//		/**
//		 * @brief Управляющий локациями
//		 */
//		LocationsManager* m_locationsManager;

//		/**
//		 * @brief Управляющий статистикой
//		 */
//		StatisticsManager* m_statisticsManager;

		/**
		 * @brief Управляющий настройками
		 */
		SettingsManager* m_settingsManager;

//		/**
//		 * @brief Управляющий импортом
//		 */
//		ImportManager* m_importManager;

//		/**
//		 * @brief Управляющий экспортом
//		 */
//		ExportManager* m_exportManager;

		/**
		 * @brief Управляющий синхронизацией
		 */
		SynchronizationManager* m_synchronizationManager;
	};
}

#endif // APPLICATIONMANAGER_H
