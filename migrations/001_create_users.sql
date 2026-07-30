CREATE TABLE IF NOT EXISTS users
(
    id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    username VARCHAR(20) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT users_username_unique UNIQUE (username),
    CONSTRAINT users_username_length
        CHECK (char_length(username) BETWEEN 3 AND 20),
    CONSTRAINT users_username_format
        CHECK (username ~ '^[A-Za-z0-9_]+$')
);