import inkcpp_py as ink


class TestWhitespaceMode:
    def test_default_collapses_runs(self, assets, generate):
        story, store, runner = generate(assets["WhitespaceMode"])
        assert runner.getline() == "A B\n"
        assert runner.num_choices() == 1
        assert runner.get_choice(0).text() == "Pick me"

    def test_keep_runs_keeps_them(self, assets, generate):
        story, store, runner = generate(assets["WhitespaceMode"])
        runner.set_whitespace_mode(ink.WhitespaceMode.KeepRuns)
        assert runner.getline() == "A    B\n"
        assert runner.num_choices() == 1
        assert runner.get_choice(0).text() == "Pick   me"

    def test_collapse_can_be_set_back(self, assets, generate):
        story, store, runner = generate(assets["WhitespaceMode"])
        runner.set_whitespace_mode(ink.WhitespaceMode.KeepRuns)
        runner.set_whitespace_mode(ink.WhitespaceMode.Collapse)
        assert runner.getline() == "A B\n"

    def test_rng_seed_is_settable(self, assets, generate):
        story, store, runner = generate(assets["WhitespaceMode"])
        runner.set_rng_seed(1337)
        assert runner.getline() == "A B\n"
