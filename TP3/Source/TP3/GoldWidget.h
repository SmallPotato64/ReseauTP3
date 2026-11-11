// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GoldWidget.generated.h"

/**
 * 
 */
UCLASS()
class TP3_API UGoldWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Met à jour l'affichage de l'or
	void SetGold(int32 NewGold);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UFUNCTION()
	void HandleGoldChanged(int32 NewGold);

	UPROPERTY()
	TObjectPtr<class ATP3Character> BoundCharacter;
};
