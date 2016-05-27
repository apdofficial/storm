#include "matrix.h"

#include "src/storage/SparseMatrix.h"

typedef storm::storage::SparseMatrix<double>::index_type entry_index;

void define_sparse_matrix(py::module& m) {

    // MatrixEntry
    py::class_<storm::storage::MatrixEntry<entry_index, double>>(m, "SparseMatrixEntry", "Entry of sparse matrix")
        .def("__str__", [](storm::storage::MatrixEntry<entry_index, double> const& entry) {
                std::stringstream stream;
                stream << entry;
                return stream.str();
            })
        //def_property threw "pointer being freed not allocated" after exiting
        .def("value", &storm::storage::MatrixEntry<entry_index, double>::getValue, "Value")
        .def("set_value", &storm::storage::MatrixEntry<entry_index, double>::setValue, py::arg("value"), "Set value")
        .def("column", &storm::storage::MatrixEntry<entry_index, double>::getColumn, "Column")
    ;
 
    py::class_<storm::storage::SparseMatrix<double>>(m, "SparseMatrix", "Sparse matrix")
        .def("__iter__", [](storm::storage::SparseMatrix<double>& matrix) {
                return py::make_iterator(matrix.begin(), matrix.end());
            }, py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def("__str__", [](storm::storage::SparseMatrix<double> const& matrix) {
                std::stringstream stream;
                stream << matrix;
                return stream.str();
            })
        .def("nr_rows", &storm::storage::SparseMatrix<double>::getRowCount, "Number of rows")
        .def("nr_columns", &storm::storage::SparseMatrix<double>::getColumnCount, "Number of columns")
        .def("nr_entries", &storm::storage::SparseMatrix<double>::getEntryCount, "Number of non-zero entries")
        .def("_row_group_indices", &storm::storage::SparseMatrix<double>::getRowGroupIndices, "Get starting rows of row groups")
        .def("get_row", [](storm::storage::SparseMatrix<double>& matrix, entry_index row) {
                return matrix.getRows(row, row+1);
            }, py::return_value_policy::reference, py::keep_alive<1, 0>(), py::arg("row"), "Get row")
        .def("get_rows", [](storm::storage::SparseMatrix<double>& matrix, entry_index start, entry_index end) {
                return matrix.getRows(start, end);
            }, py::return_value_policy::reference, py::keep_alive<1, 0>(), py::arg("row_start"), py::arg("row_end"), "Get rows from start to end")
        .def("print_row", [](storm::storage::SparseMatrix<double> const& matrix, entry_index row) {
                std::stringstream stream;
                auto rows = matrix.getRows(row, row+1);
                for (auto transition : rows) {
                    stream << transition << ", ";
                }
                return stream.str();
            })
    ;

    // Rows
    py::class_<storm::storage::SparseMatrix<double>::rows>(m, "SparseMatrixRows", "Set of rows in a sparse matrix")
        .def("__iter__", [](storm::storage::SparseMatrix<double>::rows& rows) {
                return py::make_iterator(rows.begin(), rows.end());
            }, py::keep_alive<0, 1>())
        .def("__str__", [](storm::storage::SparseMatrix<double>::rows& rows) {
                std::stringstream stream;
                for (auto transition : rows) {
                    stream << transition << ", ";
                }
                return stream.str();
            })
    ;
}
