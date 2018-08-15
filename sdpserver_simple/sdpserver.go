package main

import (
"net/http"
"fmt"
"github.com/labstack/echo"
)

// User
type User struct {
	//Name  string `json:"name" form:"name" query:"name"`
	Email string `json:"email"`
}

func main() {
	e := echo.New()
	e.GET("/", func(c echo.Context) error {
		return c.String(http.StatusOK, "Hello, World!")
		})

	e.POST("/", func(c echo.Context) (err error) {
		u := new(User)
		if err = c.Bind(u); err != nil {
			return
		} else {
			fmt.Println(u.Email)
		}
		return c.JSON(http.StatusOK, u)
		})
	e.Logger.Fatal(e.Start(":1323"))
}