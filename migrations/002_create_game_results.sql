CREATE TABLE IF NOT EXISTS game_results
(
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,

    white_user_id BIGINT NOT NULL,
    black_user_id BIGINT NOT NULL,

    winner VARCHAR(5) NOT NULL,

    created_at TIMESTAMPTZ NOT NULL
        DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT game_results_white_user_fk
        FOREIGN KEY (white_user_id)
        REFERENCES users (id),

    CONSTRAINT game_results_black_user_fk
        FOREIGN KEY (black_user_id)
        REFERENCES users (id),

    CONSTRAINT game_results_players_different
        CHECK (white_user_id <> black_user_id),

    CONSTRAINT game_results_winner_valid
        CHECK (winner IN ('white', 'black'))
);

CREATE INDEX IF NOT EXISTS
    game_results_white_user_id_idx
ON game_results (white_user_id);

CREATE INDEX IF NOT EXISTS
    game_results_black_user_id_idx
ON game_results (black_user_id);

CREATE INDEX IF NOT EXISTS
    game_results_created_at_idx
ON game_results (created_at);