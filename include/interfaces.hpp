#pragma once

class ICustomView
{
    public:
        virtual void setupUi() = 0;
        virtual void setupLayout() = 0;
        virtual void setupConnections() = 0;
};