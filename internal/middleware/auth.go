package middleware

import (
	"log"
	"net/http"
	"os"

	"github.com/gin-gonic/gin"
)

func APIKeyAuth() gin.HandlerFunc {
	return func(c *gin.Context) {
		key := c.GetHeader("X-API-Key")
		expectedKey := os.Getenv("EXPORTER_API_KEY")

		if key == "" || key != expectedKey {
			log.Printf("[AUTH] Failed access from IP: %s - Path: %s", c.ClientIP(), c.Request.URL.Path)

			c.JSON(http.StatusUnauthorized, gin.H{
				"error": "Unauthorized access",
			})
			c.Abort()
			return
		}

		c.Next()
	}
}
