// Python bindings for the Hex engine. Board/Game/AI logic is identical to
// what the doctest suite and the benchmark tool exercise -- this file is
// just the pybind11 boundary the FastAPI backend talks to.
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "hex/ai.hpp"
#include "hex/board.hpp"
#include "hex/game.hpp"

namespace py = pybind11;
using namespace hex;

PYBIND11_MODULE(hex_engine, m) {
    m.doc() = "C++ Hex board, game state machine, and AI strategies (Random / Flat Monte Carlo / MCTS-UCT)";

    py::enum_<Player>(m, "Player")
        .value("RED", Player::Red)
        .value("BLUE", Player::Blue)
        .value("NONE", Player::None);

    m.def("opponent", &opponent, py::arg("player"));

    py::class_<Board>(m, "Board")
        .def(py::init<int>(), py::arg("size"))
        .def_property_readonly("size", &Board::size)
        .def("at", &Board::at, py::arg("row"), py::arg("col"))
        .def("place", &Board::place, py::arg("row"), py::arg("col"), py::arg("player"))
        .def("remove", &Board::remove, py::arg("row"), py::arg("col"))
        .def("empty_cells", &Board::emptyCells)
        .def("is_full", &Board::isFull)
        .def("winner", &Board::winner)
        .def("winning_path", &Board::winningPath, py::arg("player"))
        .def("grid", &Board::grid);

    py::enum_<GameStatus>(m, "GameStatus").value("IN_PROGRESS", GameStatus::InProgress).value("FINISHED", GameStatus::Finished);

    py::class_<Game>(m, "Game")
        .def(py::init<int, Player>(), py::arg("board_size"), py::arg("first_to_move"))
        .def_property_readonly("board", static_cast<Board& (Game::*)()>(&Game::board),
                                py::return_value_policy::reference_internal)
        .def_property_readonly("to_move", &Game::toMove)
        .def_property_readonly("status", &Game::status)
        .def_property_readonly("winner", &Game::winner)
        .def("apply_move", &Game::applyMove, py::arg("row"), py::arg("col"), py::arg("player"));

    py::class_<AIStrategy>(m, "AIStrategy")
        // MctsAI/FlatMonteCarloAI can take a real amount of CPU time; release
        // the GIL for the call so a FastAPI worker thread running this
        // doesn't block other Python threads (e.g. other games' requests)
        // while it thinks.
        .def("select_move", &AIStrategy::selectMove, py::arg("board"), py::arg("player"),
             py::call_guard<py::gil_scoped_release>())
        .def_property_readonly("name", &AIStrategy::name);

    py::class_<RandomAI, AIStrategy>(m, "RandomAI")
        .def(py::init<>())
        .def(py::init<std::mt19937::result_type>(), py::arg("seed"));

    py::class_<FlatMonteCarloAI, AIStrategy>(m, "FlatMonteCarloAI")
        .def(py::init<int>(), py::arg("trials") = 200)
        .def(py::init<int, std::mt19937::result_type>(), py::arg("trials"), py::arg("seed"));

    py::class_<MctsAI, AIStrategy>(m, "MctsAI")
        .def(py::init<int, double>(), py::arg("iterations") = 1500,
             py::arg("exploration_constant") = 1.4142135623730951)
        .def(py::init<int, double, std::mt19937::result_type>(), py::arg("iterations"),
             py::arg("exploration_constant"), py::arg("seed"));
}
