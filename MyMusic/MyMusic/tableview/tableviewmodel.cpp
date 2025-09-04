#include "tableviewmodel.h"

TableViewModel::TableViewModel(MusicTableViewType view_type, QObject *parent)
    : QAbstractTableModel{parent}, _type(view_type)
{}
