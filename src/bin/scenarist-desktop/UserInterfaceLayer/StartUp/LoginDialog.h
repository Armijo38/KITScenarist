#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <3rd_party/Widgets/QLightBoxWidget/qlightboxdialog.h>

namespace Ui {
	class LoginDialog;
}


namespace UserInterface
{
	/**
	 * @brief Класс диалога авторизации пользователя
	 */
	class LoginDialog : public QLightBoxDialog
	{
		Q_OBJECT

	public:
		explicit LoginDialog(QWidget* _parent = 0);
		~LoginDialog();

        /**
         * @brief Email с виджета авторизации
         */
        QString loginEmail() const;

        /**
         * @brief Email с виджета регистрации
         */
        QString regEmail() const;
        void setUserName(const QString& _userName);

        /**
         * @brief Пароль с виджета авторизации
         */
        QString loginPassword() const;

        /**
         * @brief Пароль с виджета регистрации
         */
        QString regPassword() const;
        void setPassword(const QString& _password);

        /**
         * @brief Проверочный код
         */
        QString code() const;

        /**
         * @brief Тип аккаунта
         */
        QString regType() const;

		/**
		 * @brief Установить сообщение об ошибке
		 */
        void setAuthError(const QString& _error);
        void setRegisterError(const QString& _error);
        void setValidateError(const QString& _error);

        void setAuthPage();

        /**
         * @brief Установить текст для виджета проверочного кода
         */
        void showVerify();

        /**
         * @brief Установить текст для виджета восстановления пароля
         */
        void showRestore();

        /**
         * @brief Очистить окно
         */
        void clear();

    signals:
        /**
         * @brief Пользователь хочет зарегистрироваться
         */
        void registrate();

        /**
         * @brief Пользователь хочет авторизоваться
         */
        void login();

        /**
         * @brief Пользователь хочет отправить проверочный код
         */
        void verify();

        /**
         * @brief Пользователь хочет восстановить пароль
         */
        void restore();

	protected:
		/**
		 * @brief При запуске фокусируемся на поле ввода имени пользователя
		 */
		QWidget* focusedOnExec() const;

    private slots:
        /**
         * @brief Проверка, что проверочный код удовлетворяет шаблону проверочных кодов
         */
        void checkCode();

        /**
         * @brief Убрать окно ввода проверочного кода и показывать вместо него окно регистрации
         */
        void cancelVerify();

        /**
         * @brief Смена активного виджета
         */
        void switchWidget();

	private:
		/**
		 * @brief Настроить представление
		 */
		void initView();

		/**
		 * @brief Настроить соединения для формы
		 */
		void initConnections();

	private:
		Ui::LoginDialog *ui;

        /**
         * @brief Поскольку один чекбокс на 2 виджета
         *        True - окно ввода проверочного кода
         *        False - окно регистрации
         */
        bool isVerify;
	};
}

#endif // LOGINDIALOG_H
