import inkcpp_py as ink


class TestTags:
    @staticmethod
    def compare(obj):
        if isinstance(obj, ink.Runner):
            tags = obj.all_tags()
            assert obj.global_tags() == tags["global"]
            assert obj.num_global_tags() == len(tags["global"])
            assert obj.has_global_tags() == (len(tags["global"]) > 0)
            for i, tag in enumerate(tags["global"]):
                assert obj.get_global_tag(i) == tag
            assert obj.knot_tags() == tags["knot"]
            assert obj.num_knot_tags() == len(tags["knot"])
            assert obj.has_knot_tags() == (len(tags["knot"]) > 0)
            for i, tag in enumerate(tags["knot"]):
                assert obj.get_knot_tag(i) == tag
            assert obj.tags() == tags["line"]
            assert obj.num_tags() == len(tags["line"])
            assert obj.has_tags() == (len(tags["line"]) > 0)
            for i, tag in enumerate(tags["line"]):
                assert obj.get_tag(i) == tag
        elif isinstance(obj, ink.Choice):
            tags = obj.tags()
            assert obj.has_tags() == (len(tags) > 0)
            assert obj.num_tags() == len(tags)
            for i, tag in enumerate(tags):
                assert obj.get_tag(i) == tag
        else:
            raise AssertionError()

    @staticmethod
    def tags(obj):
        return {"global": [], "knot": [], "line": []} | obj

    def test_tags(self, assets, generate):
        [story, globals, runner] = generate(assets["TagsStory"])

        assert runner.all_tags() == self.tags({})
        self.compare(runner)

        expected = [
            (
                "First line has global tags only\n",
                self.tags({"global": ["global_tag"], "line": ["global_tag"]}),
                "global_tags_only",
            ),
            (
                "Second line has one tag\n",
                self.tags({"global": ["global_tag"], "line": ["tagged"]}),
                "global_tags_only",
            ),
            (
                "Third line has two tags\n",
                self.tags(
                    {"global": ["global_tag"], "line": ["tag next line", "more tags"]}
                ),
                "global_tags_only",
            ),
            (
                "Fourth line has three tags\n",
                self.tags(
                    {"global": ["global_tag"], "line": ["above", "side", "across"]}
                ),
                "global_tags_only",
            ),
            (
                "Hello\n",
                self.tags(
                    {
                        "global": ["global_tag"],
                        "knot": [
                            "knot_tag_start",
                            "second_knot_tag_start",
                            "third_knot_tag",
                        ],
                        "line": [
                            "knot_tag_start",
                            "second_knot_tag_start",
                            "third_knot_tag",
                            "output_tag_h",
                        ],
                    }
                ),
                "start",
            ),
            (
                "Second line has no tags\n",
                self.tags(
                    {
                        "global": ["global_tag"],
                        "knot": [
                            "knot_tag_start",
                            "second_knot_tag_start",
                            "third_knot_tag",
                        ],
                    }
                ),
                "start",
            ),
            [("a", []), ("b", ["choice_tag_b", "choice_tag_b_2"]), 1],
            (
                "Knot2\n",
                self.tags(
                    {
                        "global": ["global_tag"],
                        "knot": ["knot_tag_2"],
                        "line": ["knot_tag_2", "output_tag_k"],
                    }
                ),
                "knot2.sub",
            ),
            [
                ("e", []),
                (
                    "f with detail",
                    ["shared_tag", "shared_tag_2", "choice_tag", "choice_tag_2"],
                ),
                ("g", ["choice_tag_g"]),
                1,
            ],
            (
                "f and content\n",
                self.tags(
                    {
                        "global": ["global_tag"],
                        "knot": ["knot_tag_2"],
                        "line": [
                            "shared_tag",
                            "shared_tag_2",
                            "content_tag",
                            "content_tag_2",
                        ],
                    }
                ),
                "knot2.sub",
            ),
            (
                "out\n",
                self.tags(
                    {
                        "global": ["global_tag"],
                        "knot": ["knot_tag_2"],
                        "line": ["close_tag"],
                    }
                ),
                "knot2.sub",
            ),
        ]
        for (i, state) in enumerate(expected):
            if isinstance(state, tuple):
                (line, tags, label) = state
                assert runner.getline() == line, f"step {i}"
                assert runner.all_tags() == tags, f"step {i}"
                assert runner.current_knot() == ink.hash_string(label), f"step {i}"
                self.compare(runner)
            else:
                choose = state[-1]
                state = state[:-1]
                assert not runner.can_continue(), f"step {i}"
                assert runner.num_choices() == len(state), f"step {i}"
                for (text, tags), choice in zip(state, iter(runner)):
                    assert choice.text() == text, f"step {i}"
                    assert choice.tags() == tags, f"step {i}"
                    self.compare(choice)
                runner.choose(choose)
                assert runner.can_continue(), f"step {i}"
